// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <string>
using std::string;

#include <memory>
using std::unique_ptr;

#include <iostream>
using std::cout;
using std::endl;

#include <nlohmann/json/json.hpp>

#include "quadtree/tree.hpp"

using namespace geometry;
using namespace quadtree;

QuadTree::QuadTree(): QuadTree({0,0}, 1024) {}

QuadTree::QuadTree(Point _center, double _width):
    root(new Node(_center, _width, 0))
{
    root->split();
}

QuadTree::~QuadTree(){
    this->reset();
}

void QuadTree::draw_quadrant(std::ostream& sink, const string& prefix, Node* at, const string& as) const {
    sink << prefix << "[" << as << "]";
    if(at->is_leaf()){
        sink << ": " << at->get_value() << '\n';
    }else{
        sink << '\n';
        auto next_prefix = prefix + "    ";
        draw_quadrant(sink, next_prefix, at->get_northeast(), "NE");
        draw_quadrant(sink, next_prefix, at->get_northwest(), "NW");
        draw_quadrant(sink, next_prefix, at->get_southeast(), "SE");
        draw_quadrant(sink, next_prefix, at->get_southwest(), "SW");
    }

}

bool QuadTree::contains(const double x, const double y) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x;
    const double cy = bounds.center.y;
    const double dim = bounds.half_height;

    assert(bounds.half_height == bounds.half_width);

    if((x < (cx - dim)) || ((cx + dim) < x)){
        return false;
    }else if((y < (cy - dim)) || ((cy + dim) < y)){
        return false;
    }

    return true;
}

void QuadTree::deserialize(std::istream& source){
    nlohmann::json doc = nlohmann::json::parse( source,    // input
                                                nullptr,  // callback argument
                                                false);   // do not allow exceptions

    if(doc.is_discarded()){
        // in case of ANY error, discard this input
        return;
    }

    if(!(doc.is_object() && doc.contains("bounds"))){
        // 'bounds' is necessary for any well-formed tree serialization
        return;
    }else{
        auto& bounds = doc["bounds"];
        if(!(bounds.contains("x") && bounds.contains("y") && bounds.contains("width"))){
            return;
        }

        // load contents
        const Point pt = { bounds["x"].get<double>(), bounds["y"].get<double>()};
        const double width = bounds["width"].get<double>();
        quadtree::Node* new_root = new quadtree::Node(pt, width, NAN);
        root.reset(new_root);

        doc.erase("bounds");
        root->load(doc);
    }
}

void QuadTree::draw(std::ostream& sink) const{
    auto& bounds = get_bounds(); 
    sink << "====== Tree: ======\n";
    sink << "@(" << bounds.center.x << ", " << bounds.center.y << ")   |" << bounds.half_height << "|\n";

    draw_quadrant(sink, "", root.get(), "RT");
    
    sink << endl;
}

const Bounds& QuadTree::get_bounds() const {
    return root->get_bounds();
}

node_value_t QuadTree::get_default_value(const Point& at) const {
    return 99;
}

node_value_t QuadTree::interp(const Point& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if point is outside the tree, entirely
    if( ! root->contains(at)){
        return get_default_value(at);
    }

    // if the point is near-to-center if the nearest node:
    if( near.nearby(at)){
        return near.get_value();
    }

    const Point& cn = near.get_center();
    const double dx = std::copysign(1.0, (at.x - cn.x)) * 2 * near.get_bounds().half_width;
    const double dy = std::copysign(1.0, (at.y - cn.y)) * 2 * near.get_bounds().half_height;
    const Node& n2 = root->search({cn.x + dx, cn.y     });
    const Node& n3 = root->search({cn.x + dx, cn.y + dy});
    const Node& n4 = root->search({cn.x     , cn.y + dy});

    const auto& interp = near.interpolate_bilinear(at, n2, n3, n4);

    return interp;
}

void QuadTree::load(const std::vector<Point>& source){
}

node_value_t QuadTree::search(const double x, const double y) {
    const Point at(x,y);
    if(root->contains(at)){
        return root->search(at).get_value();
    }else{
        return get_default_value(at);
    }
}

void QuadTree::reset() {
    this->root.reset();
}

void QuadTree::set(double x, double y, node_value_t value) {
    /*
    // Java Implementation
    if (x < root.getX() || y < root.getY() || x > root.getX() + root.getW() || y > root.getY() + root.getH()) {
        throw new QuadTreeException("Out of bounds : (" + x + ", " + y + ")");
    }
    if (this.insert(root, new Point(x, y, value))) {
        this.count_++;
    }
    */

    /*
    // C Implementation
    quadtree_point_t *point;
    int insert_status;

    if(!(point = quadtree_point_new(x, y))) return 0;
    if(!node_contains_(tree->root, point)){
        quadtree_point_free(point);
        return 0;
    }
    
    if(!(insert_status = insert_(tree, tree->root, point, key))){
        quadtree_point_free(point);
        return 0;
    }
    if (insert_status == 1) tree->length++;
    return insert_status;
    */

}

void QuadTree::serialize(std::ostream& sink) const {
    nlohmann::json doc;

    // output the bounds once, at the root-level, and all the bounds
    // for subsequent layers follow deterministicly
    const Bounds& bounds = root->get_bounds();
    doc["bounds"] = {
        {"x", bounds.center.x},
        {"y", bounds.center.y},
        // tree is implemented & enforced as square
        {"width", 2 * bounds.half_width}
    };
    
    doc.update(root->to_json());

    // write result to output stream:
    sink << doc;
}


//     /**
//      * Traverses the tree depth-first, with quadrants being traversed in clockwise
//      * order (NE, SE, SW, NW).  The provided function will be called for each
//      * leaf node that is encountered.
//      * @param {QuadTree.Node} node The current node.
//      * @param {function(QuadTree.Node)} fn The function to call
//      *     for each leaf node. This function takes the node as an argument, and its
//      *     return value is irrelevant.
//      * @private
//      */
//     public void traverse(Node node, Func func) {
//         switch (node.getNodeType()) {
//             case LEAF:
//                 func.call(this, node);
//                 break;

//             case POINTER:
//                 this.traverse(node.getNe(), func);
//                 this.traverse(node.getSe(), func);
//                 this.traverse(node.getSw(), func);
//                 this.traverse(node.getNw(), func);
//                 break;
//         }
//     }

//     /**
//      * Finds a leaf node with the same (x, y) coordinates as the target point, or
//      * null if no point exists.
//      * @param {QuadTree.Node} node The node to search in.
//      * @param {number} x The x-coordinate of the point to search for.
//      * @param {number} y The y-coordinate of the point to search for.
//      * @return {QuadTree.Node} The leaf node that matches the target,
//      *     or null if it doesn't exist.
//      * @private
//      */
//     public Node find(Node node, double x, double y) {
//         Node resposne = null;
//         switch (node.getNodeType()) {
//             case EMPTY:
//                 break;

//             case LEAF:
//                 resposne = node.getPoint().getX() == x && node.getPoint().getY() == y ? node : null;
//                 break;

//             case POINTER:
//                 resposne = this.find(this.getQuadrantForPoint(node, x, y), x, y);
//                 break;

//             default:
//                 throw new QuadTreeException("Invalid nodeType");
//         }
//         return resposne;
//     }


// find_(quadtree_node_t* node, double x, double y) {
//   if(!node){
//     return NULL;
//   }
//   if(quadtree_node_isleaf(node)){
//     if(node->point->x == x && node->point->y == y)
//       return node->point;
//   } else if(quadtree_node_ispointer(node)){
//     quadtree_point_t test;
//     test.x = x;
//     test.y = y;
//     return find_(get_quadrant_(node, &test), x, y);
//   }

//   return NULL;
// }


std::ostream& operator<<(std::ostream& os, const quadtree::QuadTree& tree)
{
    tree.serialize(os);
    return os;
}
