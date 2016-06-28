#pragma once

#include <string>
#include <atomic>

#include "JoyoungRobot.h"
#include "Global.h"

class MovingTask;
class MovingPlan;
class MovingPlanManager;

/************************************************************************/
/* MovingTask                                                           */
/************************************************************************/

class MovingTask
{
protected:
    MovingTask(MovingPlan* pParentPlan){ ; }
public:
    virtual ~MovingTask(){ ; }
    virtual const std::string& taskName() = 0;

    virtual void envionmentVariables_Changed_vSlam() = 0;
};

/************************************************************************/
/* MovingPlan                                                           */
/************************************************************************/

class MovingPlan
{
protected:
    MovingPlan(MovingPlanManager* pPlanManager){ ; }
public:
    virtual ~MovingPlan(){ ; }
    virtual const std::string& planName() = 0;
    
    virtual MovingPlanManager* planManager() =0;

    virtual void planPlay() = 0;
    virtual void planPause() = 0;
    virtual void planStop() = 0;

    virtual MovingTask* taskCurrent() = 0;
};

/************************************************************************/
/* MovingPlanManager                                                           */
/************************************************************************/
class MovingPlanManager
{
protected:
    MovingPlanManager(JoyoungRobot* pRobot);
    virtual ~MovingPlanManager();

public:    
    JoyoungRobot* robot(){ return m_pRobot; }
    
    MovingPlan* planLaunch(const std::string& planName);

    MovingPlan* planCurrent();
    MovingTask* taskCurrent();

protected:
    JoyoungRobot* m_pRobot;
    std::atomic<MovingPlan*> m_planCurrent;

public:
    const static std::string Plan_Edge;
};

