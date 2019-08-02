// The MIT License 
// (c) 2019 Daniel Williams

#include <string>
using std::string;

#include <memory>
using std::unique_ptr;

#include <iostream>
using std::cout;
using std::endl;

#include <nlohmann/json/json.hpp>

#include "quadtree/tree.hpp"
using namespace quadtree;

QuadTree::QuadTree(): QuadTree({0,0}, 1024) {}

QuadTree::QuadTree(Point _center, double _width):
    root(new Node(_center, _width, _width))
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
        quadtree::Node* new_root = new quadtree::Node(pt, width);
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

void QuadTree::load(const std::vector<Point>& source){
}

node_value_t QuadTree::search(const double x, const double y, const node_value_t& default_value) {
    if(this->contains(x,y)){
	auto& node = *root->search(x,y);
	return node.get_value();
    }else{
	return default_value;
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


bool QuadTree::remove(double x, double y) {
    // Node node = this.find(this.root_, x, y);
    // if (node != null) {
    //     V value = node.getPoint().getValue();
    //     node.setPoint(null);
    //     node.setNodeType(NodeType.EMPTY);
    //     this.balance(node);
    //     this.count_--;
    //     return true;
    // }

    return false;
}

const Bounds& QuadTree::get_bounds() const {
    return root->get_bounds();
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


// public Point[] searchIntersect(final double xmin, final double ymin, final double xmax, final double ymax) {
    //     final List<Point> arr = new ArrayList<Point>();
    //     this.navigate(this.root_, new Func() {
            
    //         public void call(QuadTree quadTree, Node node) {
    //             Point pt = node.getPoint();
    //             if (pt.getX() < xmin || pt.getX() > xmax || pt.getY() < ymin || pt.getY() > ymax) {
    //                 // Definitely not within the polygon!
    //             } else {
    //                 arr.add(node.getPoint());
    //             }

    //         }
    //     }, xmin, ymin, xmax, ymax);
    //     return arr.toArray((Point[]) new Point[arr.size()]);
    // }

//     public Point[] searchWithin(final double xmin, final double ymin, final double xmax, final double ymax) {
//         final List<Point> arr = new ArrayList<Point>();
//         this.navigate(this.root_, new Func() {
            
//             public void call(QuadTree quadTree, Node node) {
//                 Point pt = node.getPoint();
//                 if (pt.getX() > xmin && pt.getX() < xmax && pt.getY() > ymin && pt.getY() < ymax) {
//                     arr.add(node.getPoint());
//                 }
//             }
//         }, xmin, ymin, xmax, ymax);
//         return arr.toArray((Point[]) new Point[arr.size()]);
//     }

//     public void navigate(Node node, Func func, double xmin, double ymin, double xmax, double ymax) {
//         switch (node.getNodeType()) {
//             case LEAF:
//                 func.call(this, node);
//                 break;

//             case POINTER:
//                 if (intersects(xmin, ymax, xmax, ymin, node.getNe()))
//                     this.navigate(node.getNe(), func, xmin, ymin, xmax, ymax);
//                 if (intersects(xmin, ymax, xmax, ymin, node.getSe()))
//                     this.navigate(node.getSe(), func, xmin, ymin, xmax, ymax);
//                 if (intersects(xmin, ymax, xmax, ymin, node.getSw()))
//                     this.navigate(node.getSw(), func, xmin, ymin, xmax, ymax);
//                 if (intersects(xmin, ymax, xmax, ymin, node.getNw()))
//                     this.navigate(node.getNw(), func, xmin, ymin, xmax, ymax);
//                 break;
//         }
//     }

//     private boolean intersects(double left, double bottom, double right, double top, Node node) {
//         return !(node.getX() > right ||
//                 (node.getX() + node.getW()) < left ||
//                 node.getY() > bottom ||
//                 (node.getY() + node.getH()) < top);
//     }
//     /**
//      * Clones the quad-tree and returns the new instance.
//      * @return {QuadTree} A clone of the tree.
//      */
//     public QuadTree clone() {
//         double x1 = this.root_.getX();
//         double y1 = this.root_.getY();
//         double x2 = x1 + this.root_.getW();
//         double y2 = y1 + this.root_.getH();
//         final QuadTree clone = new QuadTree(x1, y1, x2, y2);
//         // This is inefficient as the clone needs to recalculate the structure of the
//         // tree, even though we know it already.  But this is easier and can be
//         // optimized when/if needed.
//         this.traverse(this.root_, new Func() {
            
//             public void call(QuadTree quadTree, Node node) {
//                 clone.set(node.getPoint().getX(), node.getPoint().getY(), node.getPoint().getValue());
//             }
//         });


//         return clone;
//     }

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

//     /**
//      * Inserts a point into the tree, updating the tree's structure if necessary.
//      * @param {.QuadTree.Node} parent The parent to insert the point
//      *     into.
//      * @param {QuadTree.Point} point The point to insert.
//      * @return {boolean} True if a new node was added to the tree; False if a node
//      *     already existed with the correpsonding coordinates and had its value
//      *     reset.
//      * @private
//      */
//     private boolean insert(Node parent, Point point) {
//         Boolean result = false;
//         switch (parent.getNodeType()) {
//             case EMPTY:
//                 this.setPointForNode(parent, point);
//                 result = true;
//                 break;
//             case LEAF:
//                 if (parent.getPoint().getX() == point.getX() && parent.getPoint().getY() == point.getY()) {
//                     this.setPointForNode(parent, point);
//                     result = false;
//                 } else {
//                     this.split(parent);
//                     result = this.insert(parent, point);
//                 }
//                 break;
//             case POINTER:
//                 result = this.insert(
//                         this.getQuadrantForPoint(parent, point.getX(), point.getY()), point);
//                 break;

//             default:
//                 throw new QuadTreeException("Invalid nodeType in parent");
//         }
//         return result;
//     }

//     /**
//      * Converts a leaf node to a pointer node and reinserts the node's point into
//      * the correct child.
//      * @param {QuadTree.Node} node The node to split.
//      * @private
//      */
//     private void split(Node node) {
//         Point oldPoint = node.getPoint();
//         node.setPoint(null);

//         node.setNodeType(NodeType.POINTER);

//         double x = node.getX();
//         double y = node.getY();
//         double hw = node.getW() / 2;
//         double hh = node.getH() / 2;

//         node.setNw(new Node(x, y, hw, hh, node));
//         node.setNe(new Node(x + hw, y, hw, hh, node));
//         node.setSw(new Node(x, y + hh, hw, hh, node));
//         node.setSe(new Node(x + hw, y + hh, hw, hh, node));

//         this.insert(node, oldPoint);
//     }

//     /**
//      * Attempts to balance a node. A node will need balancing if all its children
//      * are empty or it contains just one leaf.
//      * @param {QuadTree.Node} node The node to balance.
//      * @private
//      */
//     private void balance(Node node) {
//         switch (node.getNodeType()) {
//             case EMPTY:
//             case LEAF:
//                 if (node.getParent() != null) {
//                     this.balance(node.getParent());
//                 }
//                 break;

//             case POINTER: {
//                 Node nw = node.getNw();
//                 Node ne = node.getNe();
//                 Node sw = node.getSw();
//                 Node se = node.getSe();
//                 Node firstLeaf = null;

//                 // Look for the first non-empty child, if there is more than one then we
//                 // break as this node can't be balanced.
//                 if (nw.getNodeType() != NodeType.EMPTY) {
//                     firstLeaf = nw;
//                 }
//                 if (ne.getNodeType() != NodeType.EMPTY) {
//                     if (firstLeaf != null) {
//                         break;
//                     }
//                     firstLeaf = ne;
//                 }
//                 if (sw.getNodeType() != NodeType.EMPTY) {
//                     if (firstLeaf != null) {
//                         break;
//                     }
//                     firstLeaf = sw;
//                 }
//                 if (se.getNodeType() != NodeType.EMPTY) {
//                     if (firstLeaf != null) {
//                         break;
//                     }
//                     firstLeaf = se;
//                 }

//                 if (firstLeaf == null) {
//                     // All child nodes are empty: so make this node empty.
//                     node.setNodeType(NodeType.EMPTY);
//                     node.setNw(null);
//                     node.setNe(null);
//                     node.setSw(null);
//                     node.setSe(null);

//                 } else if (firstLeaf.getNodeType() == NodeType.POINTER) {
//                     // Only child was a pointer, therefore we can't rebalance.
//                     break;

//                 } else {
//                     // Only child was a leaf: so update node's point and make it a leaf.
//                     node.setNodeType(NodeType.LEAF);
//                     node.setNw(null);
//                     node.setNe(null);
//                     node.setSw(null);
//                     node.setSe(null);
//                     node.setPoint(firstLeaf.getPoint());
//                 }

//                 // Try and balance the parent as well.
//                 if (node.getParent() != null) {
//                     this.balance(node.getParent());
//                 }
//             }
//             break;
//         }
//     }

//     /**
//      * Returns the child quadrant within a node that contains the given (x, y)
//      * coordinate.
//      * @param {QuadTree.Node} parent The node.
//      * @param {number} x The x-coordinate to look for.
//      * @param {number} y The y-coordinate to look for.
//      * @return {QuadTree.Node} The child quadrant that contains the
//      *     point.
//      * @private
//      */
//     private Node getQuadrantForPoint(Node parent, double x, double y) {
//         double mx = parent.getX() + parent.getW() / 2;
//         double my = parent.getY() + parent.getH() / 2;
//         if (x < mx) {
//             return y < my ? parent.getNw() : parent.getSw();
//         } else {
//             return y < my ? parent.getNe() : parent.getSe();
//         }
//     }

//     /**
//      * Sets the point for a node, as long as the node is a leaf or empty.
//      * @param {QuadTree.Node} node The node to set the point for.
//      * @param {QuadTree.Point} point The point to set.
//      * @private
//      */
//     private void setPointForNode(Node node, Point point) {
//         if (node.getNodeType() == NodeType.POINTER) {
//             throw new QuadTreeException("Can not set point for node of type POINTER");
//         }
//         node.setNodeType(NodeType.LEAF);
//         node.setPoint(point);
//     }
// }

// // /* private prototypes */
// // static int
// // split_node_(quadtree_t *tree, quadtree_node_t *node);

// static int
// insert_(quadtree_t* tree, quadtree_node_t *root, quadtree_point_t *point, void *key);

// static int
// node_contains_(quadtree_node_t *outer, quadtree_point_t *it);

// static quadtree_node_t *
// get_quadrant_(quadtree_node_t *root, quadtree_point_t *point);

// /* private implementations */
// static int
// node_contains_(quadtree_node_t *outer, quadtree_point_t *it) {
//   return outer->bounds != NULL
//       && outer->bounds->nw->x <= it->x
//       && outer->bounds->nw->y >= it->y
//       && outer->bounds->se->x >= it->x
//       && outer->bounds->se->y <= it->y;
// }

// static void
// reset_node_(quadtree_t *tree, quadtree_node_t *node){
//   if(tree->key_free != NULL) {
//     quadtree_node_reset(node, tree->key_free);
//   } else {
//     quadtree_node_reset(node, elision_);
//   }
// }

// static quadtree_node_t *
// get_quadrant_(quadtree_node_t *root, quadtree_point_t *point) {
//   if(node_contains_(root->nw, point)) return root->nw;
//   if(node_contains_(root->ne, point)) return root->ne;
//   if(node_contains_(root->sw, point)) return root->sw;
//   if(node_contains_(root->se, point)) return root->se;
//   return NULL;
// }


// static int
// split_node_(quadtree_t *tree, quadtree_node_t *node){
//   quadtree_node_t *nw;
//   quadtree_node_t *ne;
//   quadtree_node_t *sw;
//   quadtree_node_t *se;
//   quadtree_point_t *old;
//   void *key;

//   double x  = node->bounds->nw->x;
//   double y  = node->bounds->nw->y;
//   double hw = node->bounds->width / 2;
//   double hh = node->bounds->height / 2;

//                                     //minx,   miny,       maxx,       maxy
//   if(!(nw = quadtree_node_with_bounds(x,      y - hh,     x + hw,     y))) return 0;
//   if(!(ne = quadtree_node_with_bounds(x + hw, y - hh,     x + hw * 2, y))) return 0;
//   if(!(sw = quadtree_node_with_bounds(x,      y - hh * 2, x + hw,     y - hh))) return 0;
//   if(!(se = quadtree_node_with_bounds(x + hw, y - hh * 2, x + hw * 2, y - hh))) return 0;

//   node->nw = nw;
//   node->ne = ne;
//   node->sw = sw;
//   node->se = se;

//   old = node->point;
//   key   = node->key;
//   node->point = NULL;
//   node->key   = NULL;

//   return insert_(tree, node, old, key);
// }


// static quadtree_point_t*
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

// /* cribbed from the google closure library. */
// static int
// insert_(quadtree_t* tree, quadtree_node_t *root, quadtree_point_t *point, void *key) {
//   if(quadtree_node_isempty(root)){
//     root->point = point;
//     root->key   = key;
//     return 1; /* normal insertion flag */
//   } else if(quadtree_node_isleaf(root)){
//     if(root->point->x == point->x && root->point->y == point->y){
//       reset_node_(tree, root);
//       root->point = point;
//       root->key   = key;
//       return 2; /* replace insertion flag */
//     } else {
//       if(!split_node_(tree, root)){
//         return 0; /* failed insertion flag */
//       }
//       return insert_(tree, root, point, key);
//     }
//   } else if(quadtree_node_ispointer(root)){
//     quadtree_node_t* quadrant = get_quadrant_(root, point);
//     return quadrant == NULL ? 0 : insert_(tree, quadrant, point, key);
//   }
//   return 0;
// }


// /* public */
// quadtree_t*
// quadtree_new(double minx, double miny, double maxx, double maxy) {
//   quadtree_t *tree;
//   if(!(tree = malloc(sizeof(*tree))))
//     return NULL;
//   tree->root = quadtree_node_with_bounds(minx, miny, maxx, maxy);
//   if(!(tree->root))
//     return NULL;
//   tree->key_free = NULL;
//   tree->length = 0;
//   return tree;
// }


// void
// quadtree_walk(quadtree_node_t *root, void (*descent)(quadtree_node_t *node),
//                                      void (*ascent)(quadtree_node_t *node)) {
//   (*descent)(root);
//   if(root->nw != NULL) quadtree_walk(root->nw, descent, ascent);
//   if(root->ne != NULL) quadtree_walk(root->ne, descent, ascent);
//   if(root->sw != NULL) quadtree_walk(root->sw, descent, ascent);
//   if(root->se != NULL) quadtree_walk(root->se, descent, ascent);
//   (*ascent)(root);
// }


std::ostream& operator<<(std::ostream& os, const quadtree::QuadTree& tree)
{
    tree.serialize(os);
    return os;
}
