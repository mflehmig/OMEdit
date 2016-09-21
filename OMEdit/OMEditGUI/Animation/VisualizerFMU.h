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


#ifndef VISUALIZERFMU_H
#define VISUALIZERFMU_H

#include "Visualizer.h"
#include "FMUWrapper.h"
#include "Shapes.h"
#include "TimeManager.h"
#include "InputData.h"

class VisualizerFMU : public VisualizerAbstract
{
 public:
  VisualizerFMU() = delete;
  VisualizerFMU(const std::string& modelFile, const std::string& path);
  virtual ~VisualizerFMU() = default;
  VisualizerFMU(const VisualizerFMU& omvf) = delete;
  VisualizerFMU& operator=(const VisualizerFMU& omvf) = delete;


  void loadFMU(const std::string& modelFile, const std::string& path);
  void initData() override;

  void resetInputs();
  void initJoySticks();
  void initializeVisAttributes(const double time = 0.0) override;
  //void setSimulationSettings(const UserSimSettingsFMU& simSetFMU);
  const FMUWrapper* getFMU() const;
  std::shared_ptr<InputData> getInputData() const;
  fmi1_value_reference_t getVarReferencesForObjectAttribute(ShapeObjectAttribute* attr);
  int setVarReferencesInVisAttributes();
  void simulate(TimeManager& omvm) override;
  double simulateStep(const double time);
  void updateVisAttributes(const double time) override;
  void updateScene(const double time = 0.0) override;
  void updateObjectAttributeFMU(ShapeObjectAttribute* attr, fmi1_import_t* fmu);
  std::shared_ptr<InputData> getInputData();
 private:
  std::shared_ptr<FMUWrapper> _fmu;
  std::shared_ptr<SimSettingsFMU> _simSettings;
  std::shared_ptr<InputData> _inputData;
  //std::vector<Control::JoystickDevice*> _joysticks;

};


#endif // end VISUALIZERFMU_H
