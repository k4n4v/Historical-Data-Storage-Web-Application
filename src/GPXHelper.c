#include "GPXHelper.h"
#include "GPXParser.h"

/************** 
 * A1 Module 1 
 * ***********/

char* getName( xmlNode* children )
{
    char* name = (char* )malloc(sizeof(char)*1);
    name[0] = '\0';

    xmlNode *extra = NULL;
    for( extra = children; extra != NULL; extra = extra->next ){
        
        if (extra->type == XML_ELEMENT_NODE)
        {
            xmlChar *string = xmlNodeGetContent(extra);
            
            /* Get name node content */
            if( !(xmlStrcmp(extra->name, (const xmlChar *)"name")) && string != NULL ){
                char* copy = (char* )string;
                name = (char* )realloc( name, (strlen(copy)+1) );
                strcpy(name, copy);
                //printf("%s\n", temp->name);
            }
            xmlFree(string); 
        }
    }
    return name;
}



Waypoint* createWaypoint(xmlNode* node)
{
    Waypoint* temp = (Waypoint* )malloc(sizeof(Waypoint));

    char* name = getName( node->children );
    temp->name = (char* )malloc( strlen(name) +1 );
    strcpy( temp->name, name );
    free(name);

    temp->latitude = 0;
    temp->longitude = 0;
    temp->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    if (node->type == XML_ELEMENT_NODE)
    {
        xmlAttr *attr;
        for(attr = node->properties; attr != NULL; attr = attr->next){
            
            xmlNode *value = attr->children;

            //latitude
            if( !(xmlStrcmp(attr->name, (const xmlChar *)"lat")) ){
                temp->latitude = atof((char* )value->content);
                //printf( "Lat: %f\n", atof((char *)value->content) );
            
            //longitude
            }else if( !(xmlStrcmp(attr->name, (const xmlChar *)"lon")) ){
                temp->longitude = atof((char* )value->content);
                //printf( "Lon: %f\n", temp->longitude );
            }
        }
        createGPXDataList( temp->otherData, node->children );

    }
    return(temp);
}



Route* createRoute( xmlNode* node ){
    
    Route* temp = (Route* )malloc(sizeof(Route));

    char* name = getName( node->children );
    temp->name = (char* )malloc( strlen(name) +1 );
    strcpy( temp->name, name );
    free(name);

    temp->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    createWaypointList( temp->waypoints, node->children );

    temp->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    createGPXDataList( temp->otherData, node->children );

    return temp;
}



Track* createTrack( xmlNode* node )
{
    Track* temp = (Track* )malloc(sizeof(Track));
    
    char* name = getName( node->children );
    temp->name = (char* )malloc( strlen(name) +1 );
    strcpy( temp->name, name );
    free(name);

    temp->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    createSegmentList( temp->segments, node->children );

    temp->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    createGPXDataList( temp->otherData, node->children );

    return temp;
}



TrackSegment* createTrackSegment( xmlNode* children )
{
    TrackSegment* temp = (TrackSegment* )malloc(sizeof(TrackSegment));
    temp->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    
    createWaypointList(temp->waypoints, children);

    return temp;
}



void createWaypointList( List* other, xmlNode* children )
{
    xmlNode *node = NULL;
    for (node = children; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE )
        {
            if( !(xmlStrcmp(node->name, (const xmlChar *)"rtept")) || !(xmlStrcmp(node->name, (const xmlChar *)"trkpt")) ){ //rtept and trkpt are type of wpt
                Waypoint* temp = createWaypoint(node);
                insertBack(other, temp);
            }
        }
    }
}



void createGPXDataList( List* other, xmlNode* children ){

    xmlNode *node = NULL;
    for (node = children; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            xmlChar *string = xmlNodeGetContent(node);
            
            if( string != NULL && (strcmp((char* )node->name, "name") != 0) && (strcmp((char* )node->name, "rtept") != 0) && (strcmp((char* )node->name, "trkpt") != 0) && (strcmp((char* )node->name, "trkseg") != 0) ){
            
                char *copy = (char* )string;
                int len = strlen(copy);
                GPXData* temp = (GPXData* )malloc(sizeof(GPXData) + (sizeof(char)*len) +1);

                strcpy( temp->name, (char *)node->name);
                strcpy( temp->value, copy );
                insertBack(other, temp);
            }
            xmlFree(string);
        }
    }
}



void createSegmentList( List* other, xmlNode* children )
{
    xmlNode *node = NULL;
    for (node = children; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE )
        {
            if( !(xmlStrcmp(node->name, (const xmlChar *)"trkseg")) ){
                TrackSegment* trkseg = createTrackSegment(node->children);
                insertBack(other, trkseg );
            }
        }
    }
}



void createLists( GPXdoc* doc, xmlNode *children )
{
    xmlNode *node = NULL;
    for (node = children; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            if( !(xmlStrcmp(node->name, (const xmlChar *)"wpt")) ){
                
                Waypoint* wpt = createWaypoint(node);
                insertBack(doc->waypoints, wpt );
            
            }else if( !(xmlStrcmp(node->name, (const xmlChar *)"rte")) ){
                
                Route* rte = createRoute(node);
                insertBack(doc->routes, rte );

            }else if( !(xmlStrcmp(node->name, (const xmlChar *)"trk")) ){

                Track* trk = createTrack(node);
                insertBack(doc->tracks, trk); 
            }
        }
    }
}



/************** 
 * A1 Module 2 
 * ***********/

int getWptDataCount(ListIterator wptIter)
{
    int num = 0;

    void* wpt;
    while( (wpt = nextElement(&wptIter)) != NULL){
        Waypoint* temp = (Waypoint* )wpt;
        num += getLength(temp->otherData);
        
        if( temp->name[0] != '\0' ){
            num++;
        }
    }
    
    return num;
}

bool compreWptName(const void* first,const void* second)
{
    Waypoint* temp1 = (Waypoint* )first;
    char* temp2 = (char* )second;

    if( strcmp(temp1->name, temp2) == 0 ){
        return true;
    }

    return false;
}

bool compreTrkName(const void* first,const void* second)
{
    Track* temp1 = (Track* )first;
    char* temp2 = (char* )second;

    if( strcmp(temp1->name, temp2) == 0 ){
        return true;
    }

    return false;
}

bool compreRteName(const void* first,const void* second)
{
    Route* temp1 = (Route* )first;
    char* temp2 = (char* )second;

    if( strcmp(temp1->name, temp2) == 0 ){
        return true;
    }

    return false;
}