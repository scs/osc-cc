
/*	LeanCV, a computer vision library for the LeanXcam and IndXcam.
	Copyright (C) 2010 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <leancv.h>

/* Internal function: Finds the root run. */
struct LCV_REGIONS_RUN * findRoot(struct LCV_REGIONS_RUN *node)
{
	while(node->parent != NULL)
	{	
		node = node->parent;
	}
	/*printf("[%u] node's parent: [%u]\n",(uint)node,(uint)node->parent);*/
	return node;
}

/* Internal function: Unions two runs. */
void unionNodes(struct LCV_REGIONS_RUN *nodeA, struct LCV_REGIONS_RUN *nodeB)
{
	/*printf("%u and %u are to be unioned\n",(uint)nodeA,(uint)nodeB);*/
	/* TO DO: link the smaller list to the larger one */
	nodeA->parent = nodeB;
	/*printf("%u node's new parent: %u\n\n",(uint)nodeA,(uint)nodeA->parent);*/
}

/* Internal function: Finds the last run in the linked list of runs (chain) */
struct LCV_REGIONS_RUN * findLastChainNode(struct LCV_REGIONS_RUN *node)
{
	while(node->next != NULL)
	{	
		node = node->next;
	}
	return node;
}

/* Internal function: Checks the connectedness of the current detect run to the run(s) in the previous row */
void checkConnectedness(struct LCV_REGIONS_RUN *runArray, uint16 currentRun, uint16 lastRowRunOffset, uint16 lastRowRunCount)
{
	bool isOverlap = FALSE;
	struct LCV_REGIONS_RUN *rootA, *rootB, *lastNode;
	uint16 i;
	/*printf("Compare %u [%u] (%u-times, offset %u) with\n ",currentRun+1, (uint)&runArray[currentRun], lastRowRunCount, lastRowRunOffset);*/
	for(i = 0; i < lastRowRunCount; i++)
	{
		/*connex condition*/
		/*if (runArray[lastRowRunOffset+i].startColumn <= runArray[currentRun].endColumn && runArray[currentRun].startColumn <= runArray[lastRowRunOffset+i].endColumn)*/ /*4 connectedness*/
		if ((runArray[lastRowRunOffset+i].startColumn <= runArray[currentRun].endColumn+1) && (runArray[currentRun].startColumn <= runArray[lastRowRunOffset+i].endColumn+1)) /*8 connectedness */
		{
			/* TO DO: overlap condition, multiple parents handling! */
			/*printf("%u [%u] \n",lastRowRunOffset+i+1, (uint)&runArray[lastRowRunOffset+i]);*/
			rootA = findRoot(&runArray[currentRun]);
			rootB = findRoot(&runArray[lastRowRunOffset+i]);
			if (isOverlap == FALSE)
			{				
				if (rootA != rootB)
				{
					unionNodes(&runArray[currentRun],rootB);
					if (runArray[lastRowRunOffset+i].next == NULL)
						runArray[lastRowRunOffset+i].next = &runArray[currentRun];
					else
					{
						lastNode = findLastChainNode(runArray[lastRowRunOffset+i].next);
						lastNode->next = &runArray[currentRun];
					}
					/*runArray[currentRun].label = rootB->label;*/
					isOverlap = TRUE;
				}
				/*else printf("%u and %u already unioned!\n",currentRun+1,lastRowRunOffset+i+1);*/
			} 
			else 
			{
				if (rootA != rootB)
				{
					unionNodes(rootB,rootA);
					if (runArray[currentRun].next == NULL)
						runArray[currentRun].next = rootB;
					else
					{
						lastNode = findLastChainNode(runArray[currentRun].next);
						lastNode->next = rootB;
					}
				}
			}
		}
		
	}
}

/* Internal function: Labels all the regions incrementally */
uint16 LabelRegions(struct LCV_REGIONS *regions)
{
	uint16 label = 1;
	uint16 k = 0;
	struct LCV_REGIONS_RUN *root;
	uint16 i;
	
	
	/* search for roots */
	for(i = 0; i < regions->noOfRuns; i++)
	{
		if (regions->runs[i].parent == NULL)
		{
			regions->runs[i].label = label;
			regions->objects[k].root = &regions->runs[i];
			label++;
			k++;
			if(k == MAX_OBJECT_COUNT)
				break;
		}
		else
		{
			root = findRoot(&regions->runs[i]);
			regions->runs[i].label = root->label;
		}
	}
	return label-1;	
}

void lcvLabelBinary(const IplImage* img_in, struct LCV_REGIONS* regions) {
	
	if(!img_in || !regions) lcvError("Parameter is NULL");
	
	if(img_in->nChannels != 1) lcvError("Channel count must be 1");
	
	if(img_in->depth != IPL_DEPTH_8U)  lcvError("Image Depth must be IPL_DEPTH_8U");
	
	
	bool lastRowIsEmpty; 
	uint16 i,r;
	uint16 lastRowRunOffset = 0;
	uint16 lastRowRunCount = 0;
	uint16 actRowRunCount = 0;
	uint16 noOfRegions = 0;	
	uint8 *pBinImgIn = (uint8*)img_in->imageData;
	const uint16 width = img_in->width;
	const uint16 height = img_in->height;
	regions->noOfRuns = 0;
	lastRowIsEmpty = TRUE;
	
	
	/* run length encode image and initial labeling */
	for(r = 0; r < height; r++)
	{
		/*printf("--- Row #: %d\n",r);*/
		i = 0;
		/*if (r > 0)
			printf("lastRowRunCount = %u\n",actRowRunCount);*/
		lastRowRunCount = actRowRunCount;	
		actRowRunCount = 0;	
		while(regions->noOfRuns < MAX_RUN_COUNT)
		{
			/* search first foreground pixels in the row */
			while(i < width && pBinImgIn[r * width + i] == 0)
				i++;
			/* end of row? */
			if (i == width)
			{
				lastRowIsEmpty = TRUE;
				break;
			}
			/* first foreground pixel found */
			/*printf("Start Pixel \t Row: %d ; Column: %d\n",r,i);*/
			regions->runs[regions->noOfRuns].row = r;
			regions->runs[regions->noOfRuns].startColumn = i;
			regions->runs[regions->noOfRuns].label = 0;
			regions->runs[regions->noOfRuns].parent = NULL;
			regions->runs[regions->noOfRuns].next = NULL;
			/* search last foreground pixels in the row */
			while(i < width && pBinImgIn[r * width + i] == 1)
				i++;		
			/* last foreground pixel found or reached end of line, finalize run! */
			/*printf("End Pixel \t Row: %d ; Column: %d\n",r,i-1);*/
			regions->runs[regions->noOfRuns].endColumn = i-1;
			
			actRowRunCount++;
			/* check connectedness only after having scanned the first line */
			if (lastRowIsEmpty == FALSE && regions->noOfRuns > 0)
			{									
				/*regions->runs[regions->noOfRuns].label = label;
				label++;*/
				checkConnectedness(regions->runs, regions->noOfRuns, lastRowRunOffset, lastRowRunCount);	
			}		
			regions->noOfRuns++;					
			
		}
		
		if (r == 0)
			lastRowRunOffset = 0;
		else lastRowRunOffset = lastRowRunOffset+lastRowRunCount;				
		lastRowIsEmpty = FALSE;		
	}	
	/*printf("Number of detected runs: %d\n",regions->noOfRuns);*/
	noOfRegions = LabelRegions(regions);
	regions->noOfObjects = noOfRegions;
	/*printf("Number of connected components (regions): %d\n", noOfRegions);*/
	/*PrintRegionsStruct(regions);*/

}



void lcvGetRegionProperties(struct LCV_REGIONS* regions)
{
	if(!regions) lcvError("Parameter is NULL");
	
	uint16 i,k;
	uint32 AreaAccu, CxAccu, CyAccu;
	uint32 currArea, currCx, currCy, tempX;
	struct LCV_REGIONS_RUN *currentRun;
	
	/* do stuff */
	for(i = 0; i < regions->noOfObjects; i++)
	{
		currentRun = regions->objects[i].root;
		/* Init accumulators */
		AreaAccu = 0;
		CxAccu = 0;
		CyAccu = 0;
		/* Init bounding box */
		regions->objects[i].bboxTop = currentRun->row;
		regions->objects[i].bboxBottom = currentRun->row+1;
		regions->objects[i].bboxLeft = currentRun->startColumn;
		regions->objects[i].bboxRight = currentRun->endColumn;
	
		do 
		{
			currCx = 0;
			currArea = (currentRun->endColumn - currentRun->startColumn + 1);
			/* update area value */
			AreaAccu = AreaAccu + currArea;
			tempX = currentRun->startColumn;
			/* update centroid values */			
			for(k = 0; k < currArea; k++)
			{
				currCx = (currCx + tempX);
				tempX++;
			}
			currCy = currentRun->row * currArea;
			CxAccu = CxAccu + currCx;
			CyAccu = CyAccu + currCy;
			/* update bounding box values */
			regions->objects[i].bboxTop = MIN(regions->objects[i].bboxTop, currentRun->row);
			regions->objects[i].bboxBottom = MAX(regions->objects[i].bboxBottom, currentRun->row+1);
			regions->objects[i].bboxLeft = MIN(regions->objects[i].bboxLeft, currentRun->startColumn);
			regions->objects[i].bboxRight = MAX(regions->objects[i].bboxRight, currentRun->endColumn); 
			currentRun = currentRun->next;
		} while(currentRun != NULL);
		
		regions->objects[i].area = AreaAccu;
		regions->objects[i].centroidX = CxAccu / AreaAccu;
		regions->objects[i].centroidY = CyAccu / AreaAccu;
	}
	/*PrintObjectProperties(regions);*/
}
