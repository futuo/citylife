#include "GIMSGeometry.hpp"

using namespace GIMSGEOMETRY;

double distanceSquared2p(GIMSPoint *p1, GIMSPoint *p2){
    return (p2->x - p1->x) * (p2->x - p1->x)+
           (p2->y - p1->y) * (p2->y - p1->y);
}

/*returns the cosine of the angle between vectors p1-p2 and p2-p3*/
double cosine3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3){
    double SD12 = distanceSquared2p(p1, p2),
           SD13 = distanceSquared2p(p1, p3),
           SD23 = distanceSquared2p(p2, p3);

    return (SD12 + SD23 - SD13)/ (2*sqrt(SD23)*sqrt(SD12));
}

/*returns angle between vectors p1-p2 and p2-p3*/
double angle3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3){
    return acos( cosine3p(p1,p2,p3) );
}

/*Returns the squared distance between a point "p" and the closest point that
  lies in the given line segment "e". Note that the distance is squared to avoid
  possibly unnecessary square roots.*/
double distToSegmentSquared(GIMSPoint *p, GIMSEdge *e) {

    //Length of the line segment squared
    double lineSegLenSquared = (e->p1->x - e->p2->x) * (e->p1->x - e->p2->x) + 
                               (e->p1->y - e->p2->y) * (e->p1->y - e->p2->y);

    if (lineSegLenSquared == 0)
        //if true then the line segment is actually a point.
        return (p->x - e->p1->x) * (p->x - e->p1->x) + 
               (p->y - e->p1->y) * (p->y - e->p1->y);

    //dot product of (p - e->p1, e->p2 - e->p1)
    double dotProduct = (p->x - e->p1->x) * (e->p2->x - e->p1->x) + 
                        (p->y - e->p1->y) * (e->p2->y - e->p1->y);

    /*If we define the line that extends edge "e" as l(t) = e->p1 + t * (e->p2 - e->p1)
      The closest point of "p" that lies in line "l", is a point "cp" such that
      the line p(t) = p + t *(cp - p) forms a 90º degree angle with line "l".
      In order to find the value of t in l(t) for point cp, we have 
      t = [(p - e->p1).(e->p2 - e->p1)] / |e->p2 - e->p1|^2*/
    double t = dotProduct/ lineSegLenSquared;
    if (t < 0){
        //if t < 0 then cp lies beyond e->p1, and therefore e->p1 is the closest point
        return (p->x - e->p1->x) * (p->x - e->p1->x) +
               (p->y - e->p1->y) * (p->y - e->p1->y);
    }if (t > 1){
        //if t < 0 then cp lies beyond e->p2, and therefore e->p2 is the closest point
        return (p->x - e->p2->x) * (p->x - e->p2->x) +
               (p->y - e->p2->y) * (p->y - e->p2->y);
    }

    //if t is in [0,1], then we must calculate point cp (nearest_pt)
    double nearest_pt_x = e->p1->x + t * (e->p2->x - e->p1->x),
           nearest_pt_y = e->p1->y + t * (e->p2->y - e->p1->y);

    //we then return the distance between point "p" and point cp
    return (p->x - nearest_pt_x) * (p->x - nearest_pt_x) + 
           (p->y - nearest_pt_y) * (p->y - nearest_pt_y);
}


/*
-->GIMSGeometry
*/
GIMSGeometry::~GIMSGeometry (){}
/*
GIMSGeometry<--
*/

/*
-->GIMSPoint
*/
/* Returns true if the point lies inside the given bounding box */
GIMSPoint *GIMSPoint::clone () {
    return new GIMSPoint(this->x, this->y);
}

bool GIMSPoint::isInsideBox ( GIMSBoundingBox *box ) {
    /*
    double ymax = box->upperRight->y + ERR_MARGIN,
           ymin = box->lowerLeft->y  - ERR_MARGIN,
           xmax = box->upperRight->x + ERR_MARGIN,
           xmin = box->lowerLeft->x  - ERR_MARGIN;
    */
    if ( this->x <= box->upperRight->x + ERR_MARGIN &&  //xmax
            this->x >= box->lowerLeft->x  - ERR_MARGIN &&  //xmin
            this->y <= box->upperRight->y + ERR_MARGIN &&  //ymax
            this->y >= box->lowerLeft->y  - ERR_MARGIN     //ymin
       ) {
        return true;
    }
    
    return false;
}

/* Returns to which side of the vector defined by (edge->p1, edge->p2) the point
   lies. The sides are defined in the enum GIMSSide */
GIMSSide GIMSPoint::sideOf ( GIMSEdge *edge) {
    double s = (edge->p2->x - edge->p1->x) * (this->y - edge->p1->y) -
               (edge->p2->y - edge->p1->y) * (this->x - edge->p1->x);

    TODO (add some sort of ERR_MARGIN)
    return s < 0 ? RIGHT : (s > 0 ? LEFT : ALIGNED);
}

/* Assuming that this point lies on the line defined by the line segment "edge",
   this function returns true if the point lies on the given line segment. */
bool GIMSPoint::isInsideEdgeOfSameLine( GIMSEdge *edge ){
    double maxx, minx;
    if( edge->p1->x > edge->p2->x ){
        maxx = edge->p1->x;
        minx = edge->p2->x;
    }else{
        maxx = edge->p2->x;
        minx = edge->p1->x;
    }

    return this->x <= maxx + ERR_MARGIN && this->x >= minx - ERR_MARGIN;
}

GIMSGeometry *GIMSPoint::clipToBox ( GIMSBoundingBox *box ){
    if( this->isInsideBox(box) )
        return this->clone();
    else
        return NULL;
}

bool GIMSPoint::equals ( GIMSPoint *cmp ) {
    if( this->x == cmp->x && this->y == cmp->y ){
        return true;
    }
    return false;
}

GIMSPoint::GIMSPoint() {
    this->type = POINT;
    this->renderCount = 0;
}

GIMSPoint::GIMSPoint (double x, double y) {
    this->x = x;
    this->y = y;
    this->type = POINT;
    this->renderCount = 0;
}

GIMSPoint::~GIMSPoint() {
    //There's nothing nested to deallocate here.
}
/*
GIMSPoint<--
*/


/*
-->GIMSBoundingBox
*/
GIMSBoundingBox *GIMSBoundingBox::clone (){
    return new GIMSBoundingBox( this->lowerLeft->clone(), this->upperRight->clone() );
}

double GIMSBoundingBox::xlength(){
    return fabs( this->upperRight->x - this->lowerLeft->x );
}

double GIMSBoundingBox::ylength(){
    return fabs( this->upperRight->y - this->lowerLeft->y );
}

double GIMSBoundingBox::minx(){
    return this->lowerLeft->x;
}

double GIMSBoundingBox::maxx(){
    return this->upperRight->y;
}

double GIMSBoundingBox::miny(){
    return this->lowerLeft->y;
}

double GIMSBoundingBox::maxy(){
    return this->upperRight->y;
}

/*Unsupported*/
GIMSGeometry *GIMSBoundingBox::clipToBox ( GIMSBoundingBox * ){
    fprintf(stderr, "Called clipToBox on a bounding box, which is not supported.");
    exit(1);
}

GIMSBoundingBox::GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight ) {
    this->lowerLeft = lowerLeft;
    this->upperRight = upperRight;
    this->type = BOUNDINGBOX;
    this->renderCount = 0;
}

GIMSBoundingBox::~GIMSBoundingBox(){
    delete this->lowerLeft;
    delete this->upperRight;
}
/*
GIMSBoundingBox<--
*/


/*
-->GIMSEdge
*/
GIMSEdge *GIMSEdge::clone () {
    return new GIMSEdge( this->p1->clone(), this->p2->clone() );
}

GIMSEdge::GIMSEdge ( GIMSPoint *p1, GIMSPoint *p2 ){
    this->p1 = p1;
    this->p2 = p2;
    this->type = EDGE;
    this->renderCount = 0;
}

GIMSEdge::~GIMSEdge() {
    delete this->p1;
    delete this->p2;
}

GIMSGeometry *GIMSEdge::clipToBox ( GIMSBoundingBox *box ){
    
    //lowerLeft  = box->lowerLeft
    //upperRight = box->upperRight

    GIMSPoint upperLeft  = { box->lowerLeft->x , box->upperRight->y },
              lowerRight = { box->upperRight->x, box->lowerLeft->y  };

    GIMSPoint *squarePoints[] = {&upperLeft, box->upperRight, &lowerRight, box->lowerLeft};
    
    /*If all square's points lie on the same side of the line defined by the
      edge's two endpoints, then there's no intersection*/
    bool allOnSameSide = true;
    double prev = squarePoints[0]->sideOf(this);
    for ( int i = 1; i < 4; i++ ) {
        if( squarePoints[i]->sideOf(this) != prev ){
            allOnSameSide = false;
            break;
        }
    }
    if (allOnSameSide) {
        return NULL;
    }

    /*this part is reached only if the line defined by the two edge's endpoints
      intersects the square. Therefore, we do a projection on both the x and y
      axis of both the edge and the square and check for overlapings*/
    if ( this->p1->x > lowerRight.x &&
         this->p2->x > lowerRight.x    ) {
        /*edge is to the right of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->x < upperLeft.x &&
         this->p2->x < upperLeft.x     ) {
        /*edge is to the left of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->y > upperLeft.y && 
         this->p2->y > upperLeft.y    ) {
        /*edge is above of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->y < lowerRight.y && 
         this->p2->y < lowerRight.y    ) {
        /*edge is above of the rectangle*/
        return NULL;
    }
    
    return this;
}

/*returns a new edge where the endpoints are the actual intersection points with
  the bounding square */
GIMSEdge *GIMSEdge::trimToBBox (GIMSBoundingBox *box) {

    bool p1inside = this->p1->isInsideBox(box),
         p2inside = this->p2->isInsideBox(box);
         
    /*if both endpoints lie inside the square*/
    if ( p1inside && p2inside ) {
        return this->clone();
    }
    
    /*these are the square limits*/
    double ymax = box->upperRight->y,
           ymin = box->lowerLeft->y,
           xmax = box->upperRight->x,
           xmin = box->lowerLeft->x;
           
    /*if line is vertical*/
    if ( this->p1->x == this->p2->x ) {
        if ( p1inside ) {
            double int_y = this->p1->y > this->p2->y ? ymin : ymax;
            return new GIMSEdge ( this->p1->clone(), new GIMSPoint( this->p1->x, int_y ) );
            
        } else if ( p2inside ) {
            double int_y = this->p2->y > this->p1->y ? ymin : ymax;
            return new GIMSEdge ( new GIMSPoint ( this->p1->x, int_y ), this->p2->clone() );
            
        } else {
            return new GIMSEdge ( new GIMSPoint ( this->p1->x, ymin ),
                                  new GIMSPoint ( this->p1->x, ymax ) );
        }
        
    /*if line is horizontal*/
    } else if ( this->p1->y == this->p2->y ) {
        if ( p1inside ) {
            double int_x = this->p1->x > this->p2->x ? xmin : xmax;
            return new GIMSEdge ( this->p1->clone(), new GIMSPoint ( int_x, this->p1->y ) );
            
        } else if ( p2inside ) {
            double int_x = this->p2->x > this->p1->x ? xmin : xmax;
            return new GIMSEdge ( new GIMSPoint ( int_x, this->p1->y ), this->p2->clone() );
            
        } else {
            return new GIMSEdge ( new GIMSPoint ( xmin, this->p1->y ),
                                  new GIMSPoint ( xmax, this->p1->y ) );
        }
    }
    
    /*calculate the intersection points and alloc corresponding points*/
    double m = (this->p1->y - this->p2->y) / (this->p1->x - this->p2->x);
    double b = this->p1->y - m * this->p1->x;
    
    double int_ymax = (ymax - b) / m,
           int_ymin = (ymin - b) / m,
           int_xmax = m * xmax + b,
           int_xmin = m * xmin + b;
           
    GIMSPoint *top_pt    = new GIMSPoint ( int_ymax, ymax ),
              *bottom_pt = new GIMSPoint ( int_ymin, ymin ),
              *left_pt   = new GIMSPoint ( xmin, int_xmin ),
              *right_pt  = new GIMSPoint ( xmax, int_xmax );
             
    /*For each intersection point we check if it is within the square. also, to
      prevent double counting intersection points that lie in the square's
      corner we check if a similar point hasn't been counted*/
    GIMSPoint *int_p1 = p1inside ? this->p1->clone() : NULL;
    GIMSPoint *int_p2 = p2inside ? this->p2->clone() : NULL;
    
    
    GIMSPoint *int_pts[] = {top_pt, bottom_pt, left_pt, right_pt, int_p1, int_p2};
    GIMSPoint *p1 = NULL, *p2 = NULL;

    for (int i = 0; i < 6; i++) {
        if( int_pts[i] == NULL )
            continue;
        if ( !(int_pts[i]->isInsideBox(box) && int_pts[i]->isInsideEdgeOfSameLine(this)) ) {
            delete ( int_pts[i] );
            int_pts[i] = NULL;
        }else{
            if( p1 == NULL )
                p1 = int_pts[i];
            else if( int_pts[i]->x != p1->x && int_pts[i]->y != p1->y )
                p2 = int_pts[i];
            else
                delete int_pts[i];
        }
    }
    
    /*If the edge hits the square exactly on the corner, the intersection is
      a single point and therefore p2 will be set to NULL*/
    if ( p1 != NULL && p2 == NULL ) {
        return new GIMSEdge (p1, p1->clone());
    } else if ( p1 != NULL && p2 != NULL ) {
        return new GIMSEdge (p1, p2);
        
    } else {
        perror ("trimEdge called on a edge that did not intersect the square");
        exit (1);
    }
}
/*
GIMSEdge<--
*/


/*
-->GIMSGeometryList
*/
GIMSGeometryList *GIMSGeometryList::clone () {
    TODO(Implement clone function on the geometry list)
    return NULL;
}

GIMSGeometry *GIMSGeometryList::clipToBox ( GIMSBoundingBox *box ){
    GIMSGeometryList *clipped = NULL;

    for( std::list<GIMSGeometry *>::iterator it = this->list->begin();
         it != this->list->end(); it++                                 ) {
        GIMSGeometry *g = (*it)->clipToBox(box);

        if( g != NULL ){
            if( clipped == NULL ){
                clipped = new GIMSGeometryList;
            }
            clipped->list->push_back(g);
        }
    }
    if(clipped != NULL)
        clipped->id = this->id;
    return clipped;
}

GIMSGeometryList::GIMSGeometryList(){
    this->type = MIXEDLIST;
    this->list = new std::list<GIMSGeometry *>();
    this->renderCount = 0;
}

GIMSGeometryList::~GIMSGeometryList(){
    delete this->list;
}
/*
GIMSGeometryList<--
*/


/*
-->GIMSPolygon
*/
GIMSPolygon *GIMSPolygon::clone(){
    TODO(Implement clone function for polygons);
    perror("called an unimplemented function (GIMSPolygon__clone)");
    exit(-1);
}

GIMSGeometry *GIMSPolygon::clipToBox(GIMSBoundingBox *box){
    GIMSGeometryList *exterior = (GIMSGeometryList *)(this->externalRing->clipToBox(box));
    GIMSGeometryList *interior = (GIMSGeometryList *)(this->internalRings->clipToBox(box));

    if(exterior != NULL || interior != NULL ){
        GIMSGeometry *g = new GIMSPolygon( exterior, interior );
        g->id = this->id;
        return g;
    }else
        return NULL;
}

GIMSPolygon::GIMSPolygon( GIMSGeometryList *externalRing, GIMSGeometryList *internalRings ){
    this->type = POLYGON;
    this->renderCount = 0;
    if(externalRing != NULL)
        this->externalRing = externalRing;
    else
        this->externalRing = new GIMSGeometryList();
    if(internalRings != NULL)
        this->internalRings = internalRings;
    else
        this->internalRings = new GIMSGeometryList();
}

GIMSPolygon::~GIMSPolygon(){
    delete this->externalRing;
    delete this->internalRings;
}
/*
GIMSPolygon<--
*/
