#include "GPXParser.h"
#include "GPXHelper.h"

//Parts of this code are based on the libxml file example provided by the intructor

GPXdoc* createGPXdoc(char* fileName)
{
    xmlDoc *doc = NULL;
    LIBXML_TEST_VERSION

    /* Open XML file to be read */
    doc = xmlReadFile( fileName, NULL, 0 );
        if( doc == NULL ){ 
            return(NULL); //If error occured opening return NULL
        }

    xmlNode *root_node = NULL;
    root_node = xmlDocGetRootElement(doc); //Get the root element node


    /* Initiallize structs and lists for parsing */
    GPXdoc* GPXdoc_ptr = (GPXdoc *)malloc( sizeof(GPXdoc) ); //malloc for GPXdoc struct
    GPXdoc_ptr->creator = (char* )malloc(sizeof(char)*1);
    GPXdoc_ptr->version = 0;

    GPXdoc_ptr->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    GPXdoc_ptr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    GPXdoc_ptr->tracks = initializeList(&trackToString, &deleteRoute, &compareTracks);


    xmlNode *cur_node = NULL;
    for (cur_node = root_node; cur_node != NULL; cur_node = cur_node->next) //Parse through all nodes in tree
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {

            /* GPXDoc info */
            if( !(xmlStrcmp(cur_node->name, (const xmlChar *)"gpx")) )
            {
                //namespace
                strcpy(GPXdoc_ptr->namespace, (char *)cur_node->ns->href); 

                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    
                    xmlNode *value = attr->children;

                    //version
                    if( !(xmlStrcmp(attr->name, (const xmlChar *)"version")) ){
                        GPXdoc_ptr->version = atof((char *)value->content);

                    //creator 
                    } else if( !(xmlStrcmp(attr->name, (const xmlChar *)"creator")) ){

                        GPXdoc_ptr->creator = (char*)realloc( GPXdoc_ptr->creator, ((sizeof(char)*strlen((char *)value->content)) +1) );
                        strcpy(GPXdoc_ptr->creator, (char *)value->content);
                    }
                }
            }

            createLists(GPXdoc_ptr, cur_node->children);

        }
    }
    
    xmlFreeDoc(doc); //free the document pointer
    xmlCleanupParser(); //Free the global variables that may have been allocated by the parser.

return(GPXdoc_ptr);
}



void deleteGPXdoc(GPXdoc* doc)
{
    free(doc->creator);
    freeList(doc->waypoints);
    freeList(doc->routes);
    freeList(doc->tracks);
    free(doc);
}



char* GPXdocToString(GPXdoc* doc)
{
    if(doc == NULL){
        return NULL;
    }

    char* strWaypoints = toString(doc->waypoints);
    char* strRoutes = toString(doc->routes);
    char* strTrack = toString(doc->tracks);

    int len = sizeof(doc->namespace) + (sizeof(doc->version)/8) + strlen(doc->creator) + strlen(strWaypoints) + strlen(strRoutes) + strlen(strTrack) + 100;
    char* str = (char* )malloc( sizeof(char)*len );

	sprintf(str, "-GPX DOC-\nxlms: %s\nversion: %f\ncreator: %s\n\n%s\n%s\n%s\n", doc->namespace, doc->version, doc->creator, strWaypoints, strRoutes, strTrack);

    free(strWaypoints);
    free(strRoutes);
    free(strTrack);

    return str;
}




/* ******************************* List helper functions *************************** */

/* Waypoints */
void deleteWaypoint(void *data)
{
    if(data == NULL){
        return;
    }

    Waypoint* temp = (Waypoint *)data;
    free(temp->name);
    freeList(temp->otherData);
    free(temp);
}


char* waypointToString( void* data)
{
    if( data == NULL ){
        return NULL;
    }

    Waypoint* temp = (Waypoint *)data;
    
    char* other = toString(temp->otherData);

    int len = (sizeof(temp->latitude)/8) + (sizeof(temp->longitude)/8) + strlen(temp->name) + strlen(other)  + 100;
    char* str = (char*)malloc( sizeof(char) * len );
	
	sprintf(str, "lat: %f\tlon: %f\nname: %s\n%s\n", temp->latitude, temp->longitude, temp->name, other);
	
    free(other);
	return str;
}


int compareWaypoints(const void *first, const void *second)
{

    Waypoint* temp1 = (Waypoint *)first;
    Waypoint* temp2 = (Waypoint *)second;

    char* str1 = waypointToString(temp1);
    char* str2 = waypointToString(temp2);

    if( strcmp( str1, str2 ) == 0 ){
        free(str1);
        free(str2);
        return 0; //if waypoints are equal will return 0 else 1 if not equal
    }
    free(str1);
    free(str2);

    return 1;
}


/* GPXdata */
void deleteGpxData( void* data)
{
    if(data == NULL){
        return;
    }

    GPXData* temp = (GPXData* )data;
    free(temp);
}


char* gpxDataToString( void* data)
{
    if( data == NULL ){
        return NULL;
    }

    GPXData* temp = (GPXData *)data;

    int len = (strlen(temp->name)) + strlen(temp->value) + 100;
    char* str = (char*)malloc( sizeof(char) * len );
	
	sprintf(str, "Other Data: \nname: %s\tvalue: %s\n", temp->name, temp->value);

	return str;
}


int compareGpxData(const void *first, const void *second)
{
    GPXData* temp1 = (GPXData *)first;
    GPXData* temp2 = (GPXData *)second;

    char* str1 = gpxDataToString(temp1);
    char* str2 = gpxDataToString(temp2);

    if( strcmp( str1, str2 ) == 0 ){
        free(str1);
        free(str2);
        return 0; //if waypoints are equal will return 0 else 1 if not equal
    }

    free(str1);
    free(str2);
    
    return 1;
}


/* Route */
void deleteRoute(void* data)
{
    if(data == NULL){
        return;
    }

    Route* temp = (Route* )data;

    free(temp->name);
    freeList(temp->waypoints);
    freeList(temp->otherData);
    free(temp);
}


char* routeToString(void* data)
{
    if( data == NULL ){
        return NULL;
    }

    Route* temp = (Route *)data;
    
    char* wpt = toString(temp->waypoints);
    char* other = toString(temp->otherData);

    int len = strlen(temp->name) + strlen(wpt) + strlen(other) + 100;
    char* str = (char*)malloc( sizeof(char) * len );
	
	sprintf(str, "-ROUTE-\nname: %s\n%s\n%s\n", temp->name, wpt, other);
	
    free(wpt);
    free(other);

	return str;
}


int compareRoutes(const void *first, const void *second)
{
    Route* temp1 = (Route *)first;
    Route* temp2 = (Route *)second;

    char* str1 = routeToString(temp1);
    char* str2 = routeToString(temp2);

    if( strcmp( str1, str2 ) == 0 ){
        free(str1);
        free(str2);
        return 0; //if waypoints are equal will return 0 else 1 if not equal
    }
    free(str1);
    free(str2);
    
    return 1;
}



/* Track Segment */
void deleteTrackSegment(void* data)
{
    if(data == NULL){
        return;
    }

    TrackSegment* temp = (TrackSegment* )data;

    freeList(temp->waypoints);
    free(temp);
}


char* trackSegmentToString(void* data)
{
    if( data == NULL ){
        return NULL;
    }

    TrackSegment* temp = (TrackSegment *)data;
    
    char* wpt = toString(temp->waypoints);

    int len = strlen(wpt) + 100;
    char* str = (char*)malloc( sizeof(char) * len );
	
	sprintf(str, "-TrackSegment-\n%s\n", wpt);
	
    free(wpt);

	return str;
}


int compareTrackSegments(const void *first, const void *second)
{
    TrackSegment* temp1 = (TrackSegment *)first;
    TrackSegment* temp2 = (TrackSegment *)second;

    char* str1 = trackSegmentToString(temp1);
    char* str2 = trackSegmentToString(temp2);

    if( strcmp( str1, str2 ) == 0 ){
        free(str1);
        free(str2);
        return 0;
    }
    free(str1);
    free(str2);
    
    return 1;
}



/* Track */
void deleteTrack(void* data)
{
    if(data == NULL){
        return;
    }

    Track* temp = (Track* )data;

    free(temp->name);
    freeList(temp->segments);
    freeList(temp->otherData);
    free(temp);
}


char* trackToString(void* data)
{
    if( data == NULL ){
        return NULL;
    }

    Track* temp = (Track* )data;
    
    char* trkseg = toString(temp->segments);
    char* other = toString(temp->otherData);

    int len = strlen(trkseg) + strlen(other) + 100;
    char* str = (char*)malloc( sizeof(char) * len );
	
	sprintf(str, "-Track-\nname: %s\n%s\n%s\n",temp->name, trkseg, other);
	
    free(trkseg);
    free(other);

	return str;

}


int compareTracks(const void *first, const void *second)
{
    Track* temp1 = (Track *)first;
    Track* temp2 = (Track *)second;

    char* str1 = trackToString(temp1);
    char* str2 = trackToString(temp2);

    if( strcmp( str1, str2 ) == 0 ){
        free(str1);
        free(str2);
        return 0;
    }
    free(str1);
    free(str2);
    
    return 1;
}



/* ******************************* Module 2 *************************** */

int getNumWaypoints(const GPXdoc* doc)
{
    if( doc == NULL ){
        return 0;
    } 
    int num = getLength(doc->waypoints);

    return num;
}


int getNumRoutes(const GPXdoc* doc)
{
    if( doc == NULL ){
        return 0;
    } 
    int num = getLength(doc->routes);

    return num;
}


int getNumTracks(const GPXdoc* doc)
{
    if( doc == NULL ){
        return 0;
    } 
    int num = getLength(doc->tracks);

    return num;
}


int getNumSegments(const GPXdoc* doc)
{
    if( doc == NULL ){
        return 0;
    }

    int num = 0;
    ListIterator iter = createIterator(doc->tracks);

    void* elem;
    while( (elem = nextElement(&iter)) != NULL){
        Track* temp = (Track* )elem;
        num = num + getLength(temp->segments);
    } 

    return num;
}


int getNumGPXData(const GPXdoc* doc)
{
    if( doc == NULL ){
        return 0;
    }

    int num = 0;

    ListIterator wptIter = createIterator(doc->waypoints);
    ListIterator trkIter = createIterator(doc->tracks);
    ListIterator rteIter = createIterator(doc->routes);


    num += getWptDataCount(wptIter);

    void* trk;
    while( (trk = nextElement(&trkIter)) != NULL){
        Track* temp1 = (Track* )trk;
        num += getLength(temp1->otherData);

        if( temp1->name[0] != '\0' ){
            num++;
        }

        ListIterator trksegIter = createIterator(temp1->segments);

        void* trkseg;
        while( (trkseg = nextElement(&trksegIter)) != NULL){

            TrackSegment* temp2 = (TrackSegment* )trkseg;
            ListIterator trksegIter2 = createIterator(temp2->waypoints);

            num += getWptDataCount(trksegIter2);

        }
    }

    void* rte;
    while( (rte = nextElement(&rteIter)) != NULL){
        Route* temp = (Route* )rte;
        num += getLength(temp->otherData);

        ListIterator rteptIter = createIterator(temp->waypoints);
        num += getWptDataCount(rteptIter);

        if( temp->name[0] != '\0' ){
            num++;
        }
    }

    return num;
}



Waypoint* getWaypoint(const GPXdoc* doc, char* name)
{
    if( doc == NULL || name == NULL ){
        return NULL;
    }

    void* data;

    if( (data = findElement(doc->waypoints, &compreWptName, name)) != NULL ){
        return data;
    }
    
    return NULL;
}


Track* getTrack(const GPXdoc* doc, char* name)
{
    if( doc == NULL || name == NULL ){
        return NULL;
    }

    void* data;

    if( (data = findElement(doc->tracks, &compreTrkName, name)) != NULL ){
        return data;
    }
    
    return NULL;
}


Route* getRoute(const GPXdoc* doc, char* name)
{
    if( doc == NULL || name == NULL ){
        return NULL;
    }

    void* data;

    if( (data = findElement(doc->routes, &compreRteName, name)) != NULL ){
        return data;
    }
    
    return NULL;
}
