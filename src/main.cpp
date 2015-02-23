#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "PGConnection.h"
#include "Connect2Postgis.h"
#include "Geometry.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"

using namespace PMQUADTREE;
using namespace GIMS_GEOMETRY;

int total = 0;
unsigned long int incId = 1;

GIMS_Geometry *retrieveFeature ( OGRFeature *feature );

int main (int argc, char *argv[]) {

    Geometry::Connect2Postgis getGeom;
    OGRLayer *pol_layer = getGeom.GetLayerByName ("planet_osm_polygon");
    OGRLayer *pt_layer = getGeom.GetLayerByName ("planet_osm_point");
    OGRLayer *ln_layer = getGeom.GetLayerByName ("planet_osm_line");
    OGRLayer *road_layer = getGeom.GetLayerByName ("planet_osm_roads");

    OGREnvelope *envelope = new OGREnvelope;
    if ( ln_layer->GetExtent ( envelope, FALSE ) != OGRERR_NONE ) {
        perror ("could not retrieve layer envelope");
        exit (-1);
    }
    double lenx = envelope->MaxX - envelope->MinX,
           leny = envelope->MaxY - envelope->MinY,
           len  = lenx > leny ? lenx : leny;

    PMQuadTree *tree = 
        new PMQuadTree( new GIMS_BoundingBox(
            new GIMS_Point (envelope->MinX - len, envelope->MinY - len),
            new GIMS_Point (envelope->MinX + len, envelope->MinY + len)
        ));
   
    lenx = leny = len;

    OGRFeature *feature;
    GIMS_Geometry* query = NULL;
    GIMS_Geometry* aux;
    int count = 0;

    GIMS_GeometryCollection *glist = new GIMS_GeometryCollection();
    OGRLayer *layers[4] = {pt_layer, ln_layer, pol_layer, road_layer};
    for(OGRLayer *layer : layers){
        count = 0;
        while ( (feature = layer->GetNextFeature() ) != NULL) {
        
            /*if ( feature->GetFieldAsDouble(feature->GetFieldIndex("way_area")) < 5000000 ) {
                delete feature;
                continue;
            }*/
            
            aux = retrieveFeature (feature);
            
            if(aux != NULL){
                aux->id = incId++;
                glist->append(aux);
                tree->insert ( aux );
                if(!count && aux->type == POLYGON)
                    if(((GIMS_Polygon *)aux)->internalRings != NULL)
                        query = aux;
                count++;
            }
            delete feature;

            if(count >= 15)
                break;

        }
        printf("%d geoms\n", count);
        delete layer;
    }
    printf("inserted %d points.\n", total);
    tree->query = query;
    renderer = new DebRenderer();
    renderer->setScale(400.0/lenx, -400.0/leny);
    renderer->setTranslation( -envelope->MinX, -envelope->MaxY );
    renderer->renderCallback = tree;
    renderer->renderSvg("outtree.svg", 400, 400);
    renderer->mainloop(argc, argv);

    int merda;
    scanf("%d", &merda); 
    
    return 0;
}

GIMS_Geometry *retrieveFeature ( OGRFeature *feature ) {

    OGRGeometry *geometry = feature->GetGeometryRef();
    
    if (geometry->getGeometryType() == wkbPoint) { //wkbMultiPoint
        total++;
        OGRPoint *f_point = (OGRPoint *)geometry;
        GIMS_Point *point = new GIMS_Point(f_point->getX(), f_point->getY());
        return point;

    } else if (geometry->getGeometryType() == wkbLineString) { //wkbMultiLineString 

        OGRLineString *f_lineString = ( (OGRLineString *) geometry);
        int N = f_lineString->getNumPoints();
        total += N;
        GIMS_LineString *lineString = new GIMS_LineString(N);

        OGRPoint *tmp = new OGRPoint;
        for (int i = 0; i < N; i++) {
            f_lineString->getPoint (i, tmp);
            lineString->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
        }
        
        delete tmp;
        return lineString;
        
    } else if (geometry->getGeometryType() == wkbPolygon) { //wkbMultiPolygon
 
        OGRLinearRing *extRing = ( (OGRPolygon *) geometry)->getExteriorRing();
        if(extRing == NULL){
            perror("Polygon without exterior ring!\n");
            return NULL;
        }
        int N = extRing->getNumPoints();
        GIMS_Ring *exteriorRing = new GIMS_Ring(N);

        total += N;
        OGRPoint *tmp = new OGRPoint;
        for (int i = 0; i < N; i++) {
            extRing->getPoint (i, tmp);
            exteriorRing->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
        }
        
        int M = ( (OGRPolygon *) geometry)->getNumInteriorRings();
        GIMS_Polygon *polygon = new GIMS_Polygon(1,M);
        polygon->appendExternalRing(exteriorRing);

        for( int k=0; k<M; k++ ){ 
            OGRLinearRing *f_intRing = ( (OGRPolygon *) geometry)->getInteriorRing(k);
            N = f_intRing->getNumPoints();
            total += N;
            GIMS_Ring *intRing = new GIMS_Ring(N);

            for (int i = 0; i < N; i++) {
                f_intRing->getPoint (i, tmp);
                intRing->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
            }
            polygon->appendInternalRing(intRing);
        }

        delete tmp;
        return polygon;
    } else {
        perror ("unsupported type of geometry detected.");
        return NULL;
    }

}
