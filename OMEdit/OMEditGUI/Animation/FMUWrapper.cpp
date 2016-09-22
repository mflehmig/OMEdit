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


#include "FMUWrapper.h"

SimSettingsFMU::SimSettingsFMU()
                : _callEventUpdate(fmi1_false),
                  _toleranceControlled(fmi1_true),
                  _intermediateResults(fmi1_false),
                  _tstart(0.0),
                  _hdef(0.1),
                  _tend(0.1),
                  _relativeTolerance(0.001),
                  _solver(Solver::EULER_FORWARD)
{
}

void SimSettingsFMU::setTend(const fmi1_real_t t)
{
  _tend = t;
}

void SimSettingsFMU::setTstart(const fmi1_real_t t)
{
  _tstart = t;
}

void SimSettingsFMU::setHdef(const fmi1_real_t h)
{
  _hdef = h;
}

void SimSettingsFMU::setRelativeTolerance(const fmi1_real_t t)
{
  _relativeTolerance = t;
}

fmi1_real_t SimSettingsFMU::getTend() const
{
  return _tend;
}

fmi1_real_t SimSettingsFMU::getTstart() const
{
  return _tstart;
}

fmi1_real_t SimSettingsFMU::getHdef()
{
  return _hdef;
}

fmi1_real_t SimSettingsFMU::getRelativeTolerance()
{
  return _relativeTolerance;
}

fmi1_boolean_t SimSettingsFMU::getToleranceControlled() const
{
  return _toleranceControlled;
}

void SimSettingsFMU::setSolver(const Solver& solver)
{
  _solver = solver;
}

fmi1_boolean_t* SimSettingsFMU::getCallEventUpdate()
{
  return &_callEventUpdate;
}

fmi1_boolean_t SimSettingsFMU::getIntermediateResults()
{
  return _intermediateResults;
}


FMUWrapper::FMUWrapper()
    : _fmu(nullptr),
      _context(nullptr),
      _callbacks(),
      _callBackFunctions(),
      _fmuData()
{
}

FMUWrapper::~FMUWrapper()
{
  // Free memory associated with the FMUData and its context.
  if (_fmuData._states)
    delete (_fmuData._states);
  if (_fmuData._statesDer)
    delete (_fmuData._statesDer);
  if (_fmuData._eventIndicators)
    delete (_fmuData._eventIndicators);
  if (_fmuData._eventIndicatorsPrev)
    delete (_fmuData._eventIndicatorsPrev);
}



void FMUWrapper::load(const std::string& modelFile, const std::string& path)
{
  // First we need to define the callbacks and set up the context.
  _callbacks.malloc = malloc;
  _callbacks.calloc = calloc;
  _callbacks.realloc = realloc;
  _callbacks.free = free;
  _callbacks.logger = jm_default_logger;
  _callbacks.log_level = jm_log_level_debug;  // jm_log_level_error;
  _callbacks.context = 0;

  _callBackFunctions.logger = fmi1_log_forwarding;
  _callBackFunctions.allocateMemory = calloc;
  _callBackFunctions.freeMemory = free;

  #ifdef FMILIB_GENERATE_BUILD_STAMP
    //printf("Library build stamp:\n%s\n", fmilib_get_build_stamp());
   std::cout << "Library build stamp: \n" << fmilib_get_build_stamp() << std::endl;
  #endif

  _context = std::shared_ptr<fmi_import_context_t>(fmi_import_allocate_context(&_callbacks), fmi_import_free_context);

  // If the FMU is already extracted, we remove the shared object file.
  /*
  std::string sharedObjectFile(fmi_import_get_dll_path(path.c_str(), modelFile.c_str(), &_callbacks));
  if (fileExists(sharedObjectFile))
  {
    if (remove(sharedObjectFile.c_str()) != 0)
      std::cout<<"Error deleting the shared object file " + sharedObjectFile + std::string(".")<<std::endl;
    else
      std::cout<<"Shared object file " << sharedObjectFile << std::string(" deleted.")<<std::endl;
  }
  else
    std::cout<<"Shared object file " << sharedObjectFile << std::string(" does not exist.")<<std::endl;
*/
  // Unzip the FMU and pars it.
  // Unzip the FMU only once. Overwriting the dll/so file may cause a segmentation fault.


  std::string fmuFileName = path + modelFile;
  fmi_version_enu_t version = fmi_import_get_fmi_version(_context.get(), fmuFileName.c_str(), path.c_str());
  if (version != fmi_version_1_enu)
  {
    std::cout<<"Only version 1.0 is supported so far. Exiting."<<std::endl;
  }

  _fmu = std::shared_ptr<fmi1_import_t>(fmi1_import_parse_xml(_context.get(), path.c_str()), fmi1_import_free);
  if (!_fmu)
  {
    std::cout<<"Error parsing XML. Exiting."<<std::endl;

  }

  //loadFMU dll
  jm_status_enu_t status = fmi1_import_create_dllfmu(_fmu.get(), _callBackFunctions, 1);
  if (status == jm_status_error)
  {
    std::cout<<"Could not create the DLL loading mechanism(C-API test). Exiting."<<std::endl;

  }
}

void FMUWrapper::initialize(const std::shared_ptr<SimSettingsFMU> simSettings)
{
  // Initialize data
  _fmuData._hcur = simSettings->getHdef();
  _fmuData._tcur = simSettings->getTstart();

  std::cout<<"Version returned from FMU: "<< std::string(fmi1_import_get_version(_fmu.get()))<<std::endl;
  std::cout<<"Platform type returned: "<< std::string(fmi1_import_get_model_types_platform(_fmu.get()))<<std::endl;

  // Calloc everything
  _fmuData._nStates = fmi1_import_get_number_of_continuous_states(_fmu.get());
  _fmuData._nEventIndicators = fmi1_import_get_number_of_event_indicators(_fmu.get());
  std::cout<<"n_states: "<< std::to_string(_fmuData._nStates) << " " << std::to_string(_fmuData._nEventIndicators)<<std::endl;

  _fmuData._states = (fmi1_real_t*) calloc(_fmuData._nStates, sizeof(double));
  _fmuData._statesDer = (fmi1_real_t*) calloc(_fmuData._nStates, sizeof(double));
  _fmuData._eventIndicators = (fmi1_real_t*) calloc(_fmuData._nEventIndicators, sizeof(double));
  _fmuData._eventIndicatorsPrev = (fmi1_real_t*) calloc(_fmuData._nEventIndicators, sizeof(double));

  // Instantiate model
  jm_status_enu_t jmstatus = fmi1_import_instantiate_model(_fmu.get(), "Test ME model instance");
  if (jmstatus == jm_status_error)
  {
    std::cout<<"fmi1_import_instantiate_model failed. Exiting."<<std::endl;
  }

  //initialize
  _fmuData._fmiStatus = fmi1_import_set_time(_fmu.get(), simSettings->getTstart());
  try
  {
	std::cout<<"we come up to here"<<std::endl;
    _fmuData._fmiStatus = fmi1_import_initialize(_fmu.get(), simSettings->getToleranceControlled(), simSettings->getRelativeTolerance(), &_fmuData._eventInfo);
	std::cout<<"the runtime error is fixed"<<std::endl;

  }
  catch (std::exception &ex)
  {
    std::cout << __FILE__ << " : " << __LINE__ << " Exception: " << ex.what() << std::endl;
  }
  _fmuData._fmiStatus = fmi1_import_get_continuous_states(_fmu.get(), _fmuData._states, _fmuData._nStates);
  _fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu.get(), _fmuData._eventIndicatorsPrev, _fmuData._nEventIndicators);
  _fmuData._fmiStatus = fmi1_import_set_debug_logging(_fmu.get(), fmi1_false);

  // Turn on logging in FMI library.
  fmi1_import_set_debug_logging(_fmu.get(), fmi1_false);
  std::cout<<"FMU::initialize(). Finished."<<std::endl;
}


const FMUData* FMUWrapper::getFMUData() const
{
  return &_fmuData;
}

fmi1_import_t* FMUWrapper::getFMU() const
{
  return _fmu.get();
}

double FMUWrapper::getTcur() const
{
  return _fmuData._tcur;
}

void FMUWrapper::setContinuousStates()
{
  _fmuData._fmiStatus = fmi1_import_set_continuous_states(_fmu.get(), _fmuData._states, _fmuData._nStates);
}

bool FMUWrapper::checkForTriggeredEvent() const
{
  for (size_t k = 0; k < _fmuData._nEventIndicators; ++k)
  {
    if (_fmuData._eventIndicators[k] * _fmuData._eventIndicatorsPrev[k] < 0)
    {
      std::cout<<"Event occurred at "<<std::to_string(_fmuData._tcur)<<std::endl;
      return true;
    }
  }
  return false;
}

bool FMUWrapper::itsEventTime() const
{
  return (_fmuData._eventInfo.upcomingTimeEvent && _fmuData._tcur == _fmuData._eventInfo.nextEventTime);
}

void FMUWrapper::updateNextTimeStep(const fmi1_real_t hdef)
{
  if (_fmuData._eventInfo.upcomingTimeEvent)
  {
    if (_fmuData._tcur + hdef < _fmuData._eventInfo.nextEventTime)
      _fmuData._hcur = hdef;
    else
      _fmuData._hcur = _fmuData._eventInfo.nextEventTime - _fmuData._tcur;
  }
  else
    _fmuData._hcur = hdef;

  // Increase with step size
  _fmuData._tcur += _fmuData._hcur;
}

void FMUWrapper::fmi1ImportGetDerivatives()
{
  _fmuData._fmiStatus = fmi1_import_get_derivatives(_fmu.get(), _fmuData._statesDer, _fmuData._nStates);
}

void FMUWrapper::handleEvents(const fmi1_boolean_t intermediateResults)
{
  std::cout<<"Handle event at "<<std::to_string(_fmuData._tcur)<<std::endl;
  _fmuData._fmiStatus = fmi1_import_eventUpdate(_fmu.get(), intermediateResults, &_fmuData._eventInfo);
  _fmuData._fmiStatus = fmi1_import_get_continuous_states(_fmu.get(), _fmuData._states, _fmuData._nStates);
  _fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu.get(), _fmuData._eventIndicators, _fmuData._nEventIndicators);
  _fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu.get(), _fmuData._eventIndicatorsPrev, _fmuData._nEventIndicators);
}

void FMUWrapper::prepareSimulationStep(const double time)
{
  _fmuData._fmiStatus = fmi1_import_set_time(_fmu.get(), time);
  _fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu.get(), _fmuData._eventIndicators, _fmuData._nEventIndicators);
}

void FMUWrapper::updateTimes(const double simTimeEnd)
{
  if (_fmuData._tcur > simTimeEnd - _fmuData._hcur / 1e16)
  {
    _fmuData._tcur -= _fmuData._hcur;
    _fmuData._hcur = simTimeEnd - _fmuData._tcur;
    _fmuData._tcur = simTimeEnd;
  }
}

void FMUWrapper::solveSystem()
{
  _fmuData._fmiStatus = fmi1_import_get_derivatives(_fmu.get(), _fmuData._statesDer, _fmuData._nStates);
}

void FMUWrapper::doEulerStep()
{
  for (size_t k = 0; k < _fmuData._nStates; ++k)
    _fmuData._states[k] = _fmuData._states[k] + _fmuData._hcur * _fmuData._statesDer[k];
}

void FMUWrapper::completedIntegratorStep(fmi1_boolean_t* callEventUpdate)
{
  _fmuData._fmiStatus = fmi1_import_completed_integrator_step(_fmu.get(), callEventUpdate);
}

