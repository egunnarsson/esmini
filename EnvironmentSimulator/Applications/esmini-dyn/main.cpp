/* 
 * esmini - Environment Simulator Minimalistic 
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * 
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

 /*
  * This application is a minimalistic example of how to make use of the scenarioengine DLL to simply play and view OpenSCENARIO files
  * In addition to Init and Step, it shows how to retrieve the state of scenario objects.
  */


#include "osi_common.pb.h"
#include "osi_object.pb.h"
#include "osi_groundtruth.pb.h"
#include "osi_version.pb.h"

#include "stdio.h"
#include "esminiLib.hpp"
#include "CommonMini.hpp"

#define DEMONSTRATE_SENSORS 1
#define DEMONSTRATE_PARAMETER 0
#define DEMONSTRATE_DRIVER_MODEL 0
#define DEMONSTRATE_OSI 0
#define DEMONSTRATE_ROADINFO 0
#define DEMONSTRATE_THREAD 0
#define DEMONSTRATE_CALLBACK 0

#define MAX_N_OBJECTS 10
#define TIME_STEP 0.017f
#define DURATION 25
#define MAX_DETECTIONS 8

static SE_ScenarioObjectState states[MAX_N_OBJECTS];

typedef struct
{
	int counter;
} Stuff;

typedef struct
{
	void* handle;
	SE_SimpleVehicleState state;
} SimpleVehicle;

void objectCallback(SE_ScenarioObjectState* state, void *my_data)
{
	const double startTrigTime = 7.0;
	const double latDist = 3.5;
	const double duration = 4.0;
	static bool firstTime = true;
	static double latOffset0;

	Stuff* stuff = (Stuff*)my_data;
	
	printf("mydata.counter: %d\n", stuff->counter);

	if (SE_GetSimulationTime() > startTrigTime && SE_GetSimulationTime() < startTrigTime + duration)
	{
		if (firstTime)
		{
			latOffset0 = state->laneOffset;
			firstTime = false;
		}
		else 
		{
			float latOffset = (float)(latOffset0 + latDist * (SE_GetSimulationTime() - startTrigTime)/duration);
			SE_ReportObjectRoadPos(state->id, state->timestamp, state->roadId, state->laneId, latOffset, state->s, state->speed);
		}
	}
}

int main(int argc, char *argv[])
{
	Stuff stuff;
	SimpleVehicle vehicle = { 0, {0, 0, 0, 0, 0, 0} };
	char* filename = 0;

	if (!filename == 0 && argc < 2)
	{
		printf("Usage variant 1: %s <osc filename>\n", FileNameOf(argv[0]).c_str());
		printf("Usage variant 2: %s --osc <filename> [additional arguments - see esmini documentation]\n", FileNameOf(argv[0]).c_str());
	}
	else if (argc == 2)
	{
		// Single argument assumed to be osc filename
		filename = argv[1];
	}


	for (int a = 0; a < 1; a++)
	{
		stuff.counter = 0;

		if (filename)
		{
			if (SE_Init(filename, 0, 1, DEMONSTRATE_THREAD, 0) != 0)
			{
				printf("Failed to load %s", filename);
				break;
			}
		}
		else
		{
			if (SE_InitWithArgs(argc, argv) != 0)
			{
				printf("Failed to initialize the scenario with given arguments\n");
				break;
			}
		}

		// Demonstrate use of ODR query function
		printf("odr filename: %s\n", SE_GetODRFilename());

#if DEMONSTRATE_DRIVER_MODEL
		SE_ScenarioObjectState state;
		SE_GetObjectState(0, &state);
		vehicle.handle = SE_SimpleVehicleCreate(state.x, state.y, state.h, 4.0);
		SE_ViewerShowFeature((1 << 2), true);
#endif

#if DEMONSTRATE_CALLBACK
		SE_RegisterObjectCallback(0, objectCallback, (void*)&stuff);
#endif

#if DEMONSTRATE_SENSORS
		// Add four sensors around the vehicle
		SE_AddObjectSensor(0, 4.0f, 0.0f, 0.5f, 0.0f, 6.0f, 50.0f, (float)(50.0 * M_PI / 180.0), MAX_DETECTIONS);
		SE_AddObjectSensor(0, 2.0f, 1.0f, 0.5f, 1.5f, 1.0f, 20.0f, (float)(120.0 * M_PI / 180.0), MAX_DETECTIONS);
		SE_AddObjectSensor(0, 2.0f, -1.0f, 0.5f, -1.5f, 1.0f, 20.0f, (float)(120.0 * M_PI / 180.0), MAX_DETECTIONS);
		SE_AddObjectSensor(0, -1.0f, 0.0f, 0.5f, 3.14f, 5.0f, 30.0f, (float)(50.0 * M_PI / 180.0), MAX_DETECTIONS);
#endif

#if DEMONSTRATE_OSI
		osi3::GroundTruth* gt;
		SE_OpenOSISocket("127.0.0.1");
		SE_OSIFileOpen();
#endif


		for (int i = 0; i*TIME_STEP < DURATION && !(SE_GetQuitFlag() == 1); i++)
		{
#if DEMONSTRATE_PARAMETER  // try with lane_change.xosc which sets "DummyParameter"
			double number = 0;
			SE_Parameter param;
			param.value = &number;
			param.name = "DummyParameter";
			static bool triggered = false;

			if (triggered == false && SE_GetSimulationTime() > 2.5)
			{
				// Actions is triggered when DummyParameter > 10
				// In scenario lane_change.xosc the trigger will happen at simtime == 3.0 s, by setting param value = 11.0
				// Let's trig it already at 2.5 from here by setting param value = 15.0
				number = 15.0;
				SE_SetParameter(param);
				triggered = true;
			}

			SE_GetParameter(&param);
			printf("param value: %.2f\n", number);
#endif

			if (SE_Step() != 0)
			{
				return 0;
			}

			stuff.counter++;
#if DEMONSTRATE_OSI  // set to 1 to demonstrate example of how to query OSI Ground Truth

			int svSize = 0;

			SE_UpdateOSIGroundTruth();
			// Fetch and parse OSI message
			gt = (osi3::GroundTruth*)SE_GetOSIGroundTruthRaw();
			
			// Print timestamp
			printf("timestamp: %.2f\n", gt->mutable_timestamp()->seconds() +
				1E-9 * gt->mutable_timestamp()->nanos());

			// Print object id, position, orientation and velocity
			for (int i = 0; i < gt->mutable_moving_object()->size(); i++)
			{
				printf(" obj id %lld pos (%.2f, %.2f, %.2f) orientation (%.2f, %.2f, %.2f) velocity (%.2f, %.2f, %.2f) \n",
					gt->mutable_moving_object(i)->mutable_id()->value(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_position()->x(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_position()->y(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_position()->z(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_orientation()->yaw(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_orientation()->pitch(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_orientation()->roll(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_velocity()->x(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_velocity()->y(),
					gt->mutable_moving_object(i)->mutable_base()->mutable_velocity()->z()
				);
			}
#endif		

#if DEMONSTRATE_ROADINFO  // set to 1 to demonstrate example of how to query road information
			SE_RoadInfo data;
			float look_ahead_distance = 10;
			int id = 0;

			SE_GetRoadInfoAtDistance(id, look_ahead_distance, &data, 0);

			printf("Road info at %.2f meter from Vehicle %d: pos (%.2f, %.2f, %.2f) curvature %.5f (r %.2f) heading %.2f pitch %.2f\n",
				look_ahead_distance, id, data.global_pos_x, data.global_pos_y, data.global_pos_z, data.curvature, 1.0 / data.curvature,  
				data.road_heading, data.road_pitch);
#endif

#if DEMONSTRATE_SENSORS  

			printf("Detections [sensor ID, Object ids]:");
			int objList[MAX_DETECTIONS];  // make room for max nr vehicles, as specified when added sensor
			for (int j = 0; j < 4; j++)
			{
				int nHits = SE_FetchSensorObjectList(j, objList);
				for (int k = 0; k < nHits; k++)
				{
					printf(" [%d, %d]", j, objList[k]);
				}
			}
			printf("\n");
#endif

#if DEMONSTRATE_DRIVER_MODEL
			SE_RoadInfo roadInfo;
			SE_GetRoadInfoAtDistance(0, 5 + 0.5f*vehicle.state.speed, &roadInfo, 0);

			double steering = 0;
			if (fabs(roadInfo.angle) > 0.01)
			{ 
				// Steer towards target point -1 or 1
				steering = roadInfo.angle;
			}
			double speedTarget = vehicle.state.speed < 25 ? 1.0 : 0.0;
			speedTarget /= (1 + 20*fabs(steering));
			SE_SimpleVehicleControlAnalog(vehicle.handle, TIME_STEP, speedTarget, roadInfo.angle);
			SE_SimpleVehicleGetState(vehicle.handle, &vehicle.state);
			
			SE_ReportObjectPos(0, i * TIME_STEP, vehicle.state.x, vehicle.state.y, vehicle.state.z, 
				vehicle.state.h, vehicle.state.p, 0, vehicle.state.speed);
#endif

			if (DEMONSTRATE_THREAD)
			{
				if (i == (int)(0.5 * DURATION / TIME_STEP))
				{
					// Halfway through, pause the simulation for a few seconds
					// to demonstrate how camera can still move independently
					// when running viewer in a separate thread
					// Only Linux and Win supported (due to OSG and MacOS issue)
					printf("Taking a 4 sec nap - if running with threads (Win/Linux) you can move camera around meanwhile\n");
					SE_sleep(4000);
				}
				else
				{
					// Normal case, sleep until its time for next simulation step
					SE_sleep((unsigned int)(TIME_STEP * 1000));
				}
			}
		}

		SE_Close();
		SE_SimpleVehicleDelete(vehicle.handle);
	}

	return 0;
}
