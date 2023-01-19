#ifndef GPX_HELPER_H
#define GPX_HELPER_H

#include "GPXParser.h"

/************** 
 * A1
 * ***********/

char* getName( xmlNode* children );

void createWaypointList( List* other, xmlNode* children );
void createGPXDataList( List* other, xmlNode* children );
void createSegmentList( List* other, xmlNode* children );

Waypoint* createWaypoint(xmlNode* node);
Route* createRoute( xmlNode* node );
TrackSegment* createTrackSegment( xmlNode* children );
Track* createTrack( xmlNode* node );

void createLists( GPXdoc* doc, xmlNode *children );

int getWptDataCount(ListIterator wptIter);

bool compreWptName(const void* first,const void* second);
bool compreTrkName(const void* first,const void* second);
bool compreRteName(const void* first,const void* second);



#endif