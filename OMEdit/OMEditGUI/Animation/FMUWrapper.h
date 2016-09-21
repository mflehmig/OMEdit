/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 * @author Volker Waurich <volker.waurich@tu-dresden.de>
 */


#ifndef FMUWRAPPER_H
#define FMUWRAPPER_H

#include "AnimationUtil.h"
#include "InputData.h"

#include "fmilib.h"
#include <iostream>
#include <memory>
#include <map>


typedef struct
{
  fmi1_real_t* _states;
  fmi1_real_t* _statesDer;
  fmi1_real_t* _eventIndicators;
  fmi1_real_t* _eventIndicatorsPrev;
  size_t _nStates;
  size_t _nEventIndicators;
  fmi1_status_t _fmiStatus;
  fmi1_event_info_t _eventInfo;
  fmi1_real_t _tcur;
  fmi1_real_t _hcur;
} FMUData;


enum class Solver
{
  NONE = 0,
  EULER_FORWARD = 1
};

class SimSettingsFMU
{
 public:
  SimSettingsFMU();
  ~SimSettingsFMU() = default;
  SimSettingsFMU(const SimSettingsFMU& ss) = delete;
  SimSettingsFMU& operator=(const SimSettingsFMU& ss) = delete;
  void setTend(const fmi1_real_t t);
  fmi1_real_t getTend() const;
  void setTstart(const fmi1_real_t t);
  fmi1_real_t getTstart() const;
  void setHdef(const fmi1_real_t h);
  fmi1_real_t getHdef();
  void setRelativeTolerance(const fmi1_real_t t);
  fmi1_real_t getRelativeTolerance();
  fmi1_boolean_t getToleranceControlled() const;
  void setSolver(const Solver& solver);
  fmi1_boolean_t* getCallEventUpdate();
  fmi1_boolean_t getIntermediateResults();
 private:
  fmi1_boolean_t _callEventUpdate;
  fmi1_boolean_t _toleranceControlled;
  fmi1_boolean_t _intermediateResults;
  fmi1_real_t _tstart;
  fmi1_real_t _hdef;
  fmi1_real_t _tend;
  fmi1_real_t _relativeTolerance;
  Solver _solver;
};


class FMUWrapper
{
 public:

  FMUWrapper();
  ~FMUWrapper();
  FMUWrapper(const FMUWrapper&) = delete;
  FMUWrapper& operator=(const FMUWrapper&) = delete;


  void load(const std::string& modelFile, const std::string& path);
  void initialize(const std::shared_ptr<SimSettingsFMU> simSettings);
  const FMUData* getFMUData() const;
  fmi1_import_t* getFMU() const;
  double getTcur() const;
  void setContinuousStates();
  bool checkForTriggeredEvent() const;
  bool itsEventTime() const;
  void updateNextTimeStep(const fmi1_real_t hdef);
  void fmi1ImportGetDerivatives();
  void handleEvents(const fmi1_boolean_t intermediateResults);
  void prepareSimulationStep(const double time);
  void updateTimes(const double simTimeEnd);
  void solveSystem();
  void doEulerStep();
  void completedIntegratorStep(fmi1_boolean_t* callEventUpdate);

 private:
  std::shared_ptr<fmi1_import_t> _fmu;
  std::shared_ptr<fmi_import_context_t> _context;
  jm_callbacks _callbacks;
  fmi1_callback_functions_t _callBackFunctions;
  FMUData _fmuData;
};

#endif // end FMUWRAPPER_H
