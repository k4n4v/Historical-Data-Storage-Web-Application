#include "GPXParser.h"

/************** 
 * A2 
 * ***********/

xmlDoc* createLibxmlTree( GPXdoc *gpxDoc );
int validateLibxmlTree( xmlDoc *doc, char *XSDFileName );

void wptNodes( xmlNodePtr root_node, List* waypoints, char* type );
void otherDataNodes( xmlNodePtr root_node, List* otherData );
void routeNodes( xmlNodePtr root_node, List* routes );
void trackNodes( xmlNodePtr root_node, List* tracks );

bool checkConstraints( GPXdoc* doc );
bool checkWaypoints( List* waypoints );
bool checkOtherData( List* otherData );
bool checkRoutes( List* routes );
bool checkTracks( List* tracks );
bool checkSegments( List* segments );

float heaversineFormula( double lon1, double lat1, double lon2, double lat2 );
void doNothingDeleteRoute( );
void doNothingDeleteTrack( );



/************** 
 * A3 
 * ***********/

char* JSGPXInfo( char* fileName, char* gpxSchemaFile );

char* JSRouteListInfo( char* fileName );

char* JSTrackListInfo( char* fileName );

char* gpxDataToJSON( GPXData* data );
char* gpxDataListToJSON( List* dataList );
char* JSgetOtherData( char* fileName, char* component, int num );

void renameTrkRte( char* fileName, char* component, int num, char* rename );

void appendRoute( char* fileName, char* routeName );

void newGPXFile( char* filename, char* str );

char* findPathRoutes( char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta );
char* findPathTracks( char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta );

