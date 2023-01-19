#include "GPXParser.h"
#include "GPXHelper.h"
#include "GPXA2Helper.h"

/************** 
 * A2 Module 1 
 * ***********/

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile)
{
    xmlDoc *doc = NULL;
    LIBXML_TEST_VERSION

    /* Open XML file to be read */
    doc = xmlReadFile( fileName, NULL, 0 );
        if( doc == NULL ){ 
            return NULL; //If error occured opening return NULL
        }
    int result = validateLibxmlTree( doc, gpxSchemaFile );
        if( result != 0 ){
            return NULL; //not a valid gpx file
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


bool writeGPXdoc( GPXdoc* doc, char* fileName )
{
    if( doc == NULL || fileName == NULL ){
        return false;
    }

    char* needle = strstr( fileName, ".gpx" );

    if( needle == NULL ){
        return false;
    }

    xmlDoc* temp = NULL;
    temp = createLibxmlTree(doc);

    if( temp == NULL ){
        return false;
    }

    xmlSaveFormatFileEnc(fileName, temp, "UTF-8", 1);
    
    xmlCleanupParser();
    xmlMemoryDump();
    xmlFreeDoc(temp);

    return true;
}


bool validateGPXDoc( GPXdoc* gpxDoc, char* gpxSchemaFile )
{
    if( gpxDoc == NULL || gpxSchemaFile == NULL ){
        return false;
    }

    bool result1 = checkConstraints(gpxDoc);
    if( result1 == false ){
        return false;
    }

    xmlDoc* temp = NULL;
    temp = createLibxmlTree(gpxDoc);

    if( temp == NULL ){
        return false;
    }

    int result = validateLibxmlTree( temp, gpxSchemaFile );

    if( result != 0 ){
        xmlCleanupParser();
        xmlMemoryDump();
        xmlFreeDoc(temp);
        
        return false;
    }

    xmlCleanupParser();
    xmlMemoryDump();
    xmlFreeDoc(temp);

    return true; //return true if result is 0 and result1 is true
}


/************** 
 * A2 Module 2 
 * ***********/ 

float round10( float len )
{
    float num = len / 10;
    //printf("%f\n", num);
    
    float final = roundf(num);
    //printf("%f\n", final);

    final = final * 10;
    //printf("%f\n", final);

    return final;
}


float getRouteLen( const Route *rt )
{
    if( rt == NULL ){
        return 0;
    }

    float len = 0;

    ListIterator wptIter = createIterator(rt->waypoints);
    Waypoint* wpt1 = NULL;
    if( (wpt1 = nextElement(&wptIter)) != NULL ){
        
        Waypoint* wpt2 = NULL;

        //int x = 0;

        Waypoint* temp = NULL;
        while( (temp = nextElement(&wptIter)) != NULL ){

            wpt2 = temp;
            len += heaversineFormula(wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude);

            //printf("%d\n", x++);
            //printf("wpt1: %f\n", wpt1->latitude);
            //printf("wpt2: %f\n\n", wpt2->latitude);

            wpt1 = wpt2;
        }
    }
    return len;
}


float getTrackLen( const Track *tr )
{
    if( tr == NULL ){
        return 0;
    }

    float len = 0;

    Waypoint* wpt1 = NULL;
    Waypoint* wpt2 = NULL;
    Waypoint* temp = NULL;
    TrackSegment* trkseg1 = NULL;
    TrackSegment* trkseg2 = NULL;

    ListIterator trkIter = createIterator(tr->segments);

    if( (trkseg1 = nextElement(&trkIter)) != NULL ){ //check if there is a segment and get it

        ListIterator wptIter = createIterator(trkseg1->waypoints);
        if( (wpt1 = nextElement(&wptIter)) != NULL ){

            while((temp = nextElement(&wptIter)) != NULL )
            {
                wpt2 = temp;
                len += heaversineFormula(wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude);
                wpt1 = wpt2;
            }
        }

        while( (trkseg2 = nextElement(&trkIter)) != NULL ) //check for more segments
        {
            wptIter = createIterator( trkseg2->waypoints );
            if( (wpt1 = nextElement(&wptIter)) != NULL )
            {
                while((temp = nextElement(&wptIter)) != NULL )
                {
                    wpt2 = temp;
                    len += heaversineFormula(wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude);
                    wpt1 = wpt2;
                }

                //length of adjacent segment
                wpt1 = getFromBack(trkseg1->waypoints);
                wpt2 = getFromFront(trkseg2->waypoints);
                len += heaversineFormula(wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude);

                trkseg1 = trkseg2;
            }
        }
    }

    return len;
}


int numRoutesWithLength( const GPXdoc* doc, float len, float delta)
{
    if( doc == NULL || len < 0 || delta < 0 ){
        return 0;
    }

    float rteLen = 0;
    float lenDiff = 0;
    int num = 0;

    ListIterator rteIter = createIterator(doc->routes);

    Route* rte = NULL;
    while( (rte = nextElement(&rteIter)) != NULL )
    {
        rteLen = getRouteLen(rte);
        lenDiff = fabs(len - rteLen);

        if( lenDiff <= delta ){
            num++;
        }
    }

    return num;
}


int numTracksWithLength( const GPXdoc* doc, float len, float delta)
{
    if( doc == NULL || len < 0 || delta < 0 ){
        return 0;
    }

    float trkLen = 0;
    float lenDiff = 0;
    int num = 0;

    ListIterator trkIter = createIterator(doc->tracks);

    Track* trk = NULL;
    while( (trk = nextElement(&trkIter)) != NULL )
    {
        trkLen = getTrackLen(trk);
        lenDiff = fabs(len - trkLen);

        if( lenDiff <= delta ){
            num++;
        }
    }

    return num;
}


bool isLoopRoute( const Route* rt, float delta )
{
    if( rt == NULL || delta < 0 ){
        return false;
    }

    float len = 0;
    int numWpt = 0;
    ListIterator wptIter = createIterator(rt->waypoints);

    Waypoint* wpt = NULL;
    while( (wpt = nextElement(&wptIter)) != NULL ){  
        numWpt++;
    }
    
    if( numWpt != 0 ){
        Waypoint* wpt1 = getFromFront(rt->waypoints);
        Waypoint* wpt2 = getFromBack(rt->waypoints);

        len = heaversineFormula( wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude );
    }

    if( len < delta && numWpt >= 4){ //if it is a loop
        return true;
    }

    return false;
}


bool isLoopTrack( const Track* tr, float delta )
{
    if( tr == NULL || delta < 0 ){
        return false;
    }

    ListIterator segIter = createIterator(tr->segments);

    int numWpt = 0;
    float len = 0;

    TrackSegment* trkseg = NULL;
    while( (trkseg = nextElement(&segIter)) != NULL )
    {
        ListIterator wptIter = createIterator(trkseg->waypoints);

        Waypoint* wpt = NULL;
        while( (wpt = nextElement(&wptIter)) != NULL ){  
            numWpt++;
        }
    }

    ListIterator Iter = createIterator(tr->segments);
    TrackSegment* firstSeg = NULL;
    if( (firstSeg = nextElement(&Iter)) != NULL ){
        
        Waypoint* wpt1 = getFromFront(firstSeg->waypoints); //first waypoint in first segment

        TrackSegment* secondSeg = getFromBack(tr->segments); //last segment

        Waypoint* wpt2 = getFromBack(secondSeg->waypoints);

        len = heaversineFormula( wpt1->longitude, wpt1->latitude, wpt2->longitude, wpt2->latitude );
    }


    if( len < delta && numWpt >= 4){ //if it is a loop
        return true;
    }

    return false;
}


List* getRoutesBetween( const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta )
{
    if( doc == NULL ){
        return NULL;
    }

    List* routeList = initializeList(&routeToString, &doNothingDeleteRoute, &compareRoutes);

    ListIterator rteIter = createIterator(doc->routes);

    Route* rte = NULL;
    while( (rte = nextElement(&rteIter)) != NULL )
    {
        Waypoint* wpt1 = getFromFront(rte->waypoints);
        float head = heaversineFormula( wpt1->longitude, wpt1->latitude, sourceLong, sourceLat );

        Waypoint* wpt2 = getFromBack(rte->waypoints);
        float tail = heaversineFormula( wpt2->longitude, wpt2->latitude, destLong, destLat );

        if( head <= delta && tail <= delta ){
            insertBack(routeList, rte);
        }
    }

    ListIterator tempIter = createIterator(routeList);
    Route* temp = NULL;
    if( (temp = nextElement(&tempIter)) == NULL ){
        freeList(routeList);
        return NULL;
    }

    return routeList;
}


List* getTracksBetween( const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta )
{
    if( doc == NULL ){
        return NULL;
    }

    List* trackList = initializeList(&trackToString, &doNothingDeleteTrack, &compareTracks);

    ListIterator trkIter = createIterator(doc->tracks);

    Track* trk = NULL;
    while( (trk = nextElement(&trkIter)) != NULL )
    {
        TrackSegment* trkseg1 = getFromFront(trk->segments);
        TrackSegment* trkseg2 = getFromBack(trk->segments);

        Waypoint* wpt1 = getFromFront(trkseg1->waypoints);
        float head = heaversineFormula( wpt1->longitude, wpt1->latitude, sourceLong, sourceLat );

        Waypoint* wpt2 = getFromBack(trkseg2->waypoints);
        float tail = heaversineFormula( wpt2->longitude, wpt2->latitude, destLong, destLat );

        if( head <= delta && tail <= delta ){
            insertBack(trackList, trk);
        }
    }

    ListIterator tempIter = createIterator(trackList);
    Route* temp = NULL;
    if( (temp = nextElement(&tempIter)) == NULL ){
        freeList(trackList);
        return NULL;
    }

    return trackList;
}


/************** 
 * A2 Module 3 
 * ***********/ 

char* routeToJSON( const Route* rt )
{
    if( rt == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "{}");

        return str;
    }
    
    //routeName
    char* routeName;
    if( rt->name[0] != '\0' ){
        routeName = rt->name;
    }else{
        routeName = "None";
    }

    //numVal
    int numVal = 0;
    numVal = getLength(rt->waypoints);
    
    //routeLen
    float routeLen = 0;
    routeLen = getRouteLen(rt);

    
    //loopStat
    float delta = 10.0;
    char* loopStat;
    bool result = isLoopRoute(rt, delta);
    
    if( result == false ){
        loopStat = "false";
    }else{
        loopStat = "true";
    }

    int len = strlen(routeName) + sizeof(numVal) + sizeof(routeLen) + strlen(loopStat) + 100;
    char* str = (char*)malloc( sizeof(char) * len );

	sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", routeName, numVal, round10(routeLen), loopStat );
	
	return str;
}


char* trackToJSON( const Track* tr )
{
    if( tr == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "{}");

        return str;
    }

    //routeName
    char* routeName;
    if( tr->name[0] != '\0' ){
        routeName = tr->name;
    }else{
        routeName = "None";
    }

    //numVal
    int numVal = 0;
    numVal = getLength(tr->segments);


    //routeLen
    float routeLen = 0;
    routeLen = getTrackLen(tr);

    
    //loopStat
    float delta = 10.0;
    char* loopStat;
    bool result = isLoopTrack(tr, delta);
    
    if( result == false ){
        loopStat = "false";
    }else{
        loopStat = "true";
    }

    int len = strlen(routeName) + sizeof(routeLen) + strlen(loopStat) + 100;
    char* str = (char*)malloc( sizeof(char) * len );

	sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", routeName, numVal, round10(routeLen), loopStat );
	
	return str;
}


char* routeListToJSON( const List *routeList )
{   
    if( routeList == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    int listLen = getLength((List* )routeList);
    if( listLen == 0 ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    char* result = malloc( sizeof(char) * 4);
    strcpy(result, "[");

    char* holder;
    char* fetch;

    ListIterator iter = createIterator((List* )routeList);
    Route* rte = NULL;
    while( (rte = nextElement(&iter)) != NULL )
    {
        holder = malloc( sizeof(char) * strlen(result) + 4 );
        strcpy(holder, result);

        free(result);

        fetch = routeToJSON( (const Route* )rte );
        result = malloc( sizeof(char) * (strlen(fetch) + strlen(holder)) + 4);
        
        strcpy(result, holder);
        strcat(result, fetch);

        free(holder);
        free(fetch);

        strcat(result, ",");
    }

    result[strlen(result)-1] = ']';
    result[strlen(result)] = '\0';

    return result;

/*
    Node* cur_node = routeList->head;

    while(cur_node)
    {
        holder = malloc( sizeof(char) * strlen(result) + 1 );
        strcpy(holder, result);

        free(result);

        fetch = routeToJSON( (Route* )cur_node->data );
        result = malloc( sizeof(char) * (strlen(fetch) + strlen(holder)) + 4);
        
        strcpy(result, holder);
        strcat(result, fetch);

        free(holder);
        free(fetch);

        strcat(result, ",");

        cur_node = cur_node->next;
    }

    result[strlen(result)-1] = ']';
    result[strlen(result)] = '\0';

    return result;
*/
}


char* trackListToJSON( const List *trackList )
{   
    if( trackList == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    int listLen = getLength((List* )trackList);
    if( listLen == 0 ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    char* result = malloc( sizeof(char) * 4);
    strcpy(result, "[");

    char* holder;
    char* fetch;

    ListIterator iter = createIterator((List* )trackList);
    Track* trk = NULL;
    while( (trk = nextElement(&iter)) != NULL )
    {
        holder = malloc( sizeof(char) * strlen(result) + 4 );
        strcpy(holder, result);

        free(result);

        fetch = trackToJSON( (const Track* )trk );
        result = malloc( sizeof(char) * (strlen(fetch) + strlen(holder)) + 4);
        
        strcpy(result, holder);
        strcat(result, fetch);

        free(holder);
        free(fetch);

        strcat(result, ",");
    }

    result[strlen(result)-1] = ']';
    result[strlen(result)] = '\0';

    return result;
}


char* GPXtoJSON( const GPXdoc* gpx )
{
    if( gpx == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    //ver
    //gpx->version

    //crVal
    //gpx->creator

    //numW
    int wpt = getNumWaypoints(gpx);

    //numR
    int rte = getNumRoutes(gpx);

    //numT
    int trk = getNumTracks(gpx);

    int len = sizeof(gpx->version) + strlen(gpx->creator) + sizeof(wpt) + sizeof(rte) + sizeof(trk) + 100;
    char* str = malloc( sizeof(char) * len );
	sprintf(str, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, wpt, rte, trk );

    return str;
}



/****************************** 
 * A2 Module 3 Bonus Functions
 * ****************************/


void addWaypoint( Route* rt, Waypoint* pt )
{
    if( rt != NULL && pt != NULL  ){
        insertBack( rt->waypoints, pt );
    }
}


void addRoute( GPXdoc* doc, Route* rt )
{
    if( doc != NULL && rt != NULL  ){
        insertBack( doc->routes, rt );
    }
}


GPXdoc* JSONtoGPX( const char* gpxString )
{
    if( gpxString == NULL ){
        return NULL;
    }

    //make a copy of the original string
    char str[strlen(gpxString)];
    strcpy(str, gpxString);
    //printf("%s\n", str);

    GPXdoc* doc = malloc(sizeof(GPXdoc));

    //waypoints, routes, tracks lists
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteRoute, &compareTracks);
    
    //namespace
    char* namespace = "http://www.topografix.com/GPX/1/1";
    strcpy( doc->namespace, namespace );

    //version
    char* tok = strtok(str, ",");
    //printf("%s\n", tok);
    char* versionStr = strchr(str, ':');
    //printf("%s\n", versionStr);
    versionStr += 1; //remove collan from string
    //printf("%s\n", versionStr);
   
    double version = atof(versionStr);
    doc->version = version;
    //printf("%f\n", doc->version);

    //creator
    tok = strtok(NULL, ",");
    //printf("%s\n", tok);
    char* creatorStr = strchr(tok, ':');
    //printf("%s\n", creatorStr);
    creatorStr += 2;
    //printf("%s\n", creatorStr);
    creatorStr[strlen(creatorStr)-2] = '\0';
    //printf("%s\n", creatorStr);

    doc->creator = malloc( (sizeof(char) * strlen(creatorStr)) +1 );
    strcpy(doc->creator, creatorStr);
    //printf("%s\n", doc->creator);

    return doc;
}


Waypoint* JSONtoWaypoint( const char* gpxString )
{
    if( gpxString == NULL ){
        return NULL;
    }

    //make a copy of the original string
    char str[strlen(gpxString)];
    strcpy(str, gpxString);
    //printf("%s\n", str);

    Waypoint* wpt = malloc(sizeof(Waypoint));
    
    //name
    wpt->name = malloc( sizeof(char) * 5);
    strcpy(wpt->name, "\0");

    //otherData list
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    //latVal
    char* tok = strtok(str, ",");
    //printf("%s\n", tok);
    char* lat = strchr(str, ':');
    //printf("%s\n", lat);
    lat += 1; //remove collan from string
    //printf("%s\n", lat);
   
    wpt->latitude = atof(lat);
    //printf("%f\n", wpt->latitude);

    //lonVal
    tok = strtok(NULL, ",");
    //printf("%s\n", tok);
    char* lon = strchr(tok, ':');
    //printf("%s\n", lon);
    lon += 1; //remove collan from string
    lon[strlen(lon)-1] = '\0';
    //printf("%s\n", lon);

    wpt->longitude = atof(lon);
    //printf("%f\n", wpt->longitude);

    return wpt;
}


Route* JSONtoRoute( const char* gpxString )
{
    if( gpxString == NULL ){
        return NULL;
    }

    //make a copy of the original string
    char str[strlen(gpxString)];
    strcpy(str, gpxString);
    //printf("%s\n", str);

    Route* rte = malloc(sizeof(Route));
    
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    //name
    char* name = strchr(str, ':');
    //printf("%s\n", name);
    name += 2;
    //printf("%s\n", name);
    name[strlen(name)-2] = '\0';
    //printf("%s\n", name);

    rte->name = malloc( (sizeof(char) * strlen(name)) +1 );
    strcpy(rte->name, name);

    return rte;
}