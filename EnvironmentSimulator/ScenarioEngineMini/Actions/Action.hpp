#pragma once

#include "OSCPrivateAction.hpp"
#include "Cars.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <math.h>

class Action
{
public:
	Action(OSCPrivateAction &privateAction, Cars &cars, std::vector<int> storyId, std::vector<std::string> &actionEntities);

	// Public methods
	void ExecuteAction(double simulationTime, double timeStep);
	void setStartTime(double simulationTime);
	void setStartAction();
	bool getStartAction();
	bool getActionCompleted();
	std::vector<int> getStoryId();
	std::string getActionType();
	std::string getActionName();

private:

	// Private methods
	void identifyActionType(OSCPrivateAction privateAction);
	void executeSinusoidal(double simulationTime);
	void executeSpeedRate(double simulationTime, double timeStep);
	void executeSpeedStep();
	void executePositionLane();
	void executePositionRoute();
	void executeFollowRoute();
	void executeMeeting();

	// Help methods
	int sign(int value);

	// Constructor variables
	OSCPrivateAction privateAction;
	Cars * carsPtr;
	std::vector<int> storyId;
	std::vector<std::string> actionEntities;
	std::string actionName;

	// General class variables
	std::string actionType;
	bool startAction;
	bool actionCompleted;
	double startTime;
	bool firstRun;

	// Sinusoidal
	std::string targetObject;
	double targetValue;
	double time;
	double f;
	std::vector<double> initialOffsets;
	bool laneChange;

	std::vector<double> aT;
	double tT;

	// Speed
	double speedRate;
	double speedTarget;

	// Meeting
	roadmanager::Position ownTargetPos;
	roadmanager::Position relativeTargetPos;
	std::string mode;
	std::string object;
	double offsetTime;
	std::string continuous;

	// Follow Route
	roadmanager::Route* route;
};
