#include "GPXA2Helper.h"

/************** 
 * A2 Module 1 
 * ***********/

xmlDoc* createLibxmlTree( GPXdoc *gpxDoc )
{
    xmlDoc *doc = NULL;
    xmlNodePtr root_node = NULL;

    LIBXML_TEST_VERSION;

    doc = xmlNewDoc(BAD_CAST "1.0"); //Creates a new document
    root_node = xmlNewNode(NULL, BAD_CAST "gpx"); //create new node called "gpx"
    xmlDocSetRootElement(doc, root_node); //set it as a root node

    //namespace
    xmlNsPtr nameSpace;
    nameSpace = xmlNewNs( root_node, BAD_CAST gpxDoc->namespace , NULL );
    xmlSetNs( root_node, nameSpace );

    //version
    char version[20];
    sprintf( version, "%.1f", gpxDoc->version );
    xmlNewProp( root_node, BAD_CAST "version", BAD_CAST version ); 

    //creator 
    xmlNewProp( root_node, BAD_CAST "creator", BAD_CAST gpxDoc->creator ); 

    //waypoints
    wptNodes( root_node, gpxDoc->waypoints, "wpt" );
    
    //routes
    routeNodes( root_node, gpxDoc->routes );
    
    //tracks
    trackNodes(root_node, gpxDoc->tracks);

    return doc;
} 


void wptNodes( xmlNodePtr root_node, List* waypoints, char* type )
{

    xmlNodePtr node = NULL;
    ListIterator wptIter = createIterator(waypoints);
    Waypoint* wpt = NULL;

    while( (wpt = nextElement(&wptIter)) != NULL){ //loops till end of list
        
        //add wpt node to tree
        node = xmlNewChild(root_node, NULL, BAD_CAST type, NULL); 

        //name
        if( wpt->name[0] != '\0' ){
            xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST wpt->name);
        }

        //lat
        char latitude[30];
        sprintf( latitude, "%f", wpt->latitude );
        xmlNewProp(node, BAD_CAST "lat", BAD_CAST latitude);

        //lon
        char longitude[30];
        sprintf( longitude, "%f", wpt->longitude );
        xmlNewProp(node, BAD_CAST "lon", BAD_CAST longitude);

        //other data
        otherDataNodes(node, wpt->otherData);
    }
}


void otherDataNodes( xmlNodePtr root_node, List* otherData )
{
    ListIterator otherDataIter = createIterator(otherData);
    GPXData* other = NULL;

    while( (other = nextElement(&otherDataIter)) != NULL){ //loops till end of list
    
        //add otherData node to tree
        xmlNewChild(root_node, NULL, BAD_CAST other->name, BAD_CAST other->value); 
    }
}


void routeNodes( xmlNodePtr root_node, List* routes )
{
    xmlNodePtr node = NULL;
    ListIterator rteIter = createIterator(routes);
    Route* rte = NULL;

    while( (rte = nextElement(&rteIter)) != NULL) //loops till end of list
    {
        //add rte node to tree
        node = xmlNewChild(root_node, NULL, BAD_CAST "rte", NULL); 

        //name
        if( rte->name[0] != '\0' ){
            xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST rte->name);
        }

        //otherData
        otherDataNodes( node, rte->otherData );

        //waypoints
        wptNodes(node, rte->waypoints, "rtept");
    }
}


void trackNodes( xmlNodePtr root_node, List* tracks )
{
    xmlNodePtr node = NULL;
    xmlNodePtr node2 = NULL;
    ListIterator trkIter = createIterator(tracks);
    Track* trk = NULL;

    while( (trk = nextElement(&trkIter)) != NULL ) //loops till end of list
    {
        //add trk node to tree
        node = xmlNewChild(root_node, NULL, BAD_CAST "trk", NULL); 

        //name
        if( trk->name[0] != '\0' ){
            xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST trk->name);
        }

        //otherData
        otherDataNodes( node, trk->otherData );

        //trkseg
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg = NULL;
        while( (trkseg = nextElement(&trksegIter)) != NULL )
        {
            node2 = xmlNewChild(node, NULL, BAD_CAST "trkseg", NULL);
            wptNodes(node2, trkseg->waypoints, "trkpt");
        }
    }
}


int validateLibxmlTree( xmlDoc *doc, char *XSDFileName )
{
	xmlSchemaPtr schema = NULL;
	xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(XSDFileName); 

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt); //validate XSD file
    xmlSchemaFreeParserCtxt(ctxt);


    xmlSchemaValidCtxtPtr ctx;
    int ret;
    ctx = xmlSchemaNewValidCtxt(schema);

    xmlSchemaSetValidErrors(ctx, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctx, doc); //returns 0 if valid xml tree
    xmlSchemaFreeValidCtxt(ctx);

    if( schema != NULL ){
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return ret;
}


bool checkConstraints( GPXdoc* doc )
{
    bool result = true;

    //namespace
    if( doc->namespace == NULL || doc->namespace[0] == '\0' ){
        return false;
    }

    //version
    if( doc->version == 0 ){
        return false;
    }

    //creator
    if( doc->creator == NULL || doc->creator[0] == '\0' ){
        return false;
    }

    //waypoints
    result = checkWaypoints(doc->waypoints);
    if( result == false ){
        return false;
    }

    //routes
    result = checkRoutes( doc->routes );
    if( result == false ){
        return false;
    }

    //tracks
    result = checkTracks(doc->tracks);
    if( result == false ){
        return false;
    }

    return true;
}


bool checkWaypoints( List* waypoints )
{
    if( waypoints == NULL ){
        return false;
    }

    ListIterator wptIter = createIterator(waypoints);
    Waypoint* wpt = NULL;
    while( (wpt = nextElement(&wptIter)) != NULL )
    {
        //name
        if( wpt->name == NULL ){
            return false;
        }

        //latitude
        if( wpt->latitude == 0 ){
            return false;
        }

        //longitude
        if( wpt->longitude == 0 ){
            return false;
        }

        //otherData list
        bool result = checkOtherData( wpt->otherData );
        if( result == false){
            return false;
        }
    }

    return true;
}


bool checkOtherData( List* otherData )
{
    if( otherData == NULL ){
        return false;
    }

    ListIterator otherDataIter = createIterator(otherData);
    GPXData* other = NULL;
    while( (other = nextElement(&otherDataIter)) != NULL )
    {
        //name
        if( other->name == NULL || other->name[0] == '\0'){
            return false;
        }

        //value
        if( other->value == NULL || other->value[0] == '\0'){
            return false;
        }
    }
    return true;
}


bool checkRoutes( List* routes )
{
    if( routes == NULL ){
        return false;
    }

    bool result = true;

    ListIterator rteIter = createIterator(routes);
    Route* rte = NULL;
    while( (rte = nextElement(&rteIter)) != NULL )
    {
        //name
        if( rte->name == NULL ){
            return false;
        }

        //waypoints
        result = checkWaypoints(rte->waypoints);
        if( result == false ){
            return false;
        }

        //otherData
        result = checkOtherData(rte->otherData);
        if( result == false ){
            return false;
        }
    }

    return true;
}


bool checkTracks( List* tracks )
{
    if( tracks == NULL ){
        return false;
    }

    bool result = true;

    ListIterator trkIter = createIterator(tracks);
    Track* trk = NULL;
    while( (trk = nextElement(&trkIter)) != NULL )
    {
        //name
        if( trk->name == NULL ){
            return false;
        }

        //segments
        result = checkSegments( trk->segments );
        if( result == false ){
            return false;
        }

        //otherData
        result = checkOtherData( trk->otherData );
        if( result == false ){
            return false;
        }
    }

    return true;
}


bool checkSegments( List* segments )
{
    if( segments == NULL ){
        return false;
    }

    bool result = true;

    ListIterator segIter = createIterator(segments);
    TrackSegment* trkseg = NULL;
    while( (trkseg = nextElement(&segIter)) != NULL )
    {
        result = checkWaypoints(trkseg->waypoints);
        if( result == false ){
            return false;
        }
    }
    
    return true;
}


/************** 
 * A2 Module 2
 * ***********/

float heaversineFormula( double lon1, double lat1, double lon2, double lat2 )
{
    //phi represents lat
    //lambda represents lon

    float radius = 6371e3;
    float phi1 = lat1 * M_PI / 180;
    float phi2 = lat2 * M_PI / 180;

    float deltaPhi = (lat2 - lat1) * M_PI / 180;
    float deltaLambda = (lon2 - lon1) * M_PI / 180;

    float a = sinf(deltaPhi/2) * sinf(deltaPhi/2) + cosf(phi1) * cosf(phi2) * sinf(deltaLambda/2) * sinf(deltaLambda/2);
    float c = 2 * atan2f(sqrt(a), sqrtf(1-a));
    
    float d = radius * c; 

    return fabs(d);
}


void doNothingDeleteRoute( )
{

}


void doNothingDeleteTrack( )
{

}

/************** 
 * A3 
 * ***********/

char* JSGPXInfo( char* fileName, char* gpxSchemaFile )
{
    GPXdoc* doc = createValidGPXdoc(fileName, gpxSchemaFile);
    if( doc == NULL ){
        return "invalid";
    }

    char* str = GPXtoJSON(doc);
    deleteGPXdoc(doc);

    return str;
}


char* JSRouteListInfo( char* fileName )
{
    GPXdoc* doc = createGPXdoc(fileName);
    if( doc == NULL ){
        return NULL;
    }

    char* str = routeListToJSON(doc->routes);
    deleteGPXdoc(doc);

    return str;
}


char* JSTrackListInfo( char* fileName )
{
    GPXdoc* doc = createGPXdoc(fileName);
    if( doc == NULL ){
        return NULL;
    }

    char* str = trackListToJSON(doc->tracks);
    deleteGPXdoc(doc);

    return str;
}

char* gpxDataToJSON( GPXData* data )
{
    if( data == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "{}");

        return str;
    }

    //dataName
    char* dataName;
    if( data->name[0] != '\0' ){
        dataName = data->name;
    }else{
        dataName = "None";
    }

    //dataVal
    char* dataVal;
    if( data->value[0] != '\0' ){
        dataVal = data->value;
    }else{
        dataVal = "None";
    }

    int len = strlen(dataName) + strlen(dataVal) + 100;
    char* str = (char*)malloc( sizeof(char) * len );

	sprintf(str, "{\"name\":\"%s\",\"value\":\"%s\"}", dataName, dataVal);
	
	return str;
}



char* gpxDataListToJSON( List* dataList )
{
    if( dataList == NULL ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    int listLen = getLength((List* )dataList);
    if( listLen == 0 ){
        char* str = malloc(sizeof(char) * 5);
        strcpy(str, "[]");

        return str;
    }

    char* result = malloc( sizeof(char) * 4);
    strcpy(result, "[");

    char* holder;
    char* fetch;

    ListIterator iter = createIterator((List* )dataList);
    GPXData* data = NULL;
    while( (data = nextElement(&iter)) != NULL )
    {
        holder = malloc( sizeof(char) * strlen(result) + 4 );
        strcpy(holder, result);

        free(result);

        fetch = gpxDataToJSON( data );
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


char* JSgetOtherData( char* fileName, char* component, int num )
{

    GPXdoc* doc = createGPXdoc(fileName);
    char* str;
    
    if( strcmp( component, "Route") == 0 ){

        ListIterator iter = createIterator((List* )doc->routes);
        Route* data = NULL;

        for( int i=0; i<num; i++ ){
            data = nextElement(&iter);
        }

        str = gpxDataListToJSON(data->otherData);
    
    }else if( strcmp( component, "Track") == 0 ){

        ListIterator iter = createIterator((List* )doc->tracks);
        Track* data = NULL;

        for( int i=0; i <num; i++ ){
            data = nextElement(&iter);
        }

        str = gpxDataListToJSON(data->otherData);

    }
    for(int i=0; i<strlen(str); i++){
        if( str[i] == '\n' ){
            str[i] = ' ';
        }
    }
        
    deleteGPXdoc(doc);
    return str;
}


void renameTrkRte( char* fileName, char* component, int num, char* rename )
{

    GPXdoc* doc = createGPXdoc(fileName);
    
    if( strcmp( component, "Route") == 0 ){

        ListIterator iter = createIterator((List* )doc->routes);
        Route* data = NULL;

        for( int i=0; i<num; i++ ){
            data = nextElement(&iter);
        }

        data->name = malloc((sizeof(char)*strlen(rename)) + 10);
        strcpy(data->name, rename); 
        writeGPXdoc( doc, fileName );
    
    }else if( strcmp( component, "Track") == 0 ){

        ListIterator iter = createIterator((List* )doc->tracks);
        Track* data = NULL;

        for( int i=0; i <num; i++ ){
            data = nextElement(&iter);
        }

        data->name = malloc((sizeof(char)*strlen(rename)) + 10);
        strcpy(data->name, rename); 
        writeGPXdoc( doc, fileName );
    }
        
    deleteGPXdoc(doc);
}


void appendRoute( char* fileName, char* routeName )
{
    GPXdoc* doc = createGPXdoc(fileName);

    Route* rte = malloc(sizeof(Route));

    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    rte->name = malloc((sizeof(char) * strlen(routeName)) + 10);

    strcpy(rte->name, routeName);

    addRoute(doc, rte);

    writeGPXdoc(doc, fileName);
} 


void newGPXFile( char* filename, char* str )
{
    GPXdoc* doc = JSONtoGPX(str);

    writeGPXdoc(doc, filename);
}


char* findPathRoutes( char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta )
{
    GPXdoc* doc = createGPXdoc(fileName);

    List* path = getRoutesBetween( doc, sourceLat, sourceLong, destLat, destLong, delta );

    char* str = routeListToJSON(path);

    return str;
}

char* findPathTracks( char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta )
{
    GPXdoc* doc = createGPXdoc(fileName);

    List* path = getTracksBetween( doc, sourceLat, sourceLong, destLat, destLong, delta );

    char* str = trackListToJSON(path);

    return str;
}
