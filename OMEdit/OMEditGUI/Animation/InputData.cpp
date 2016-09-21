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


#include "InputData.h"


const inputKey keys_real[4] = { JOY_1_X, JOY_1_Y, JOY_2_X, JOY_2_Y };
const inputKey keys_bool[4] = { KEY_W, KEY_A, KEY_S, KEY_D };

InputData::InputData()
    : _data(),
      _keyToInputMap(),
      _keyboardToKeyMap()
{
}

InputData::InputData(const InputData& ipd)
    : _data(ipd._data),
      _keyToInputMap(ipd._keyToInputMap),
      _keyboardToKeyMap(ipd._keyboardToKeyMap)
{
}


void InputData::initializeInputs(fmi1_import_t* fmu)
{
  // Get pointer to all variables.
  fmi1_import_variable_list_t* allVariables = fmi1_import_get_variable_list(fmu);
  // Define comparison function pointer.
  int (*causalityCheck)(fmi1_import_variable_t* vl, void* enumIdx);
  causalityCheck = &causalityEqual;
  int (*baseTypeCheck)(fmi1_import_variable_t* vl, void* refBaseType);
  baseTypeCheck = &baseTypeEqual;

  // Get all variables per type.
  fmi1_causality_enu_t causalityType = fmi1_causality_enu_t::fmi1_causality_enu_input;
  fmi1_import_variable_list_t* allInputs = fmi1_import_filter_variables(allVariables, causalityCheck, static_cast<void*>(&causalityType));
  fmi1_base_type_enu_t baseType = fmi1_base_type_enu_t::fmi1_base_type_real;
  fmi1_import_variable_list_t* realInputs = fmi1_import_filter_variables(allInputs, baseTypeCheck, static_cast<void*>(&baseType));
  baseType = fmi1_base_type_enu_t::fmi1_base_type_int;
  fmi1_import_variable_list_t* integerInputs = fmi1_import_filter_variables(allInputs, baseTypeCheck, static_cast<void*>(&baseType));
  baseType = fmi1_base_type_enu_t::fmi1_base_type_bool;
  fmi1_import_variable_list_t* booleanInputs = fmi1_import_filter_variables(allInputs, baseTypeCheck, static_cast<void*>(&baseType));
  baseType = fmi1_base_type_enu_t::fmi1_base_type_str;
  fmi1_import_variable_list_t* stringInputs = fmi1_import_filter_variables(allInputs, baseTypeCheck, static_cast<void*>(&baseType));

  // All value references per type.
  _data._vrReal = fmi1_import_get_value_referece_list(realInputs);
  _data._vrInteger = fmi1_import_get_value_referece_list(integerInputs);
  _data._vrBoolean = fmi1_import_get_value_referece_list(booleanInputs);
  _data._vrString = fmi1_import_get_value_referece_list(stringInputs);

  // The number of inputs per type.
  _data.setNumReal(fmi1_import_get_variable_list_size(realInputs));
  _data.setNumInteger(fmi1_import_get_variable_list_size(integerInputs));
  _data.setNumBoolean(fmi1_import_get_variable_list_size(booleanInputs));
  _data.setNumString(fmi1_import_get_variable_list_size(stringInputs));

  // The variable names.
  getVariableNames(realInputs, _data.getNumReal(), _data._namesReal);
  getVariableNames(integerInputs, _data.getNumInteger(), _data._namesInteger);
  getVariableNames(booleanInputs, _data.getNumBoolean(), _data._namesBool);
  getVariableNames(stringInputs, _data.getNumString(), _data._namesString);

  // Initialize attributes for all real inputs
  _data._attrReal = new AttributesReal[_data.getNumReal()];

  // Get attributes for all real inputs from modeldescription
  for (size_t r = 0; r < _data.getNumReal(); ++r)
  {
    fmi1_import_real_variable_t* var = fmi1_import_get_variable_as_real(fmi1_import_get_variable(realInputs, r));
    _data._attrReal[r]._max = fmi1_import_get_real_variable_max(var);
    _data._attrReal[r]._min = fmi1_import_get_real_variable_min(var);
    _data._attrReal[r]._start = fmi1_import_get_real_variable_start(var);
    _data._attrReal[r]._nominal = fmi1_import_get_real_variable_nominal(var);
  }

  initializeHelper();
}


void InputData::initializeHelper()
{
  std::cout<<"Number of Reals: "<<std::to_string(_data._namesReal.size())<<std::endl;
  std::cout<<"Number of Integers: "<<std::to_string(_data._namesInteger.size())<<std::endl;
  std::cout<<"Number of Booleans: "<<std::to_string(_data._namesBool.size())<<std::endl;
  std::cout<<"Number of Strings: "<<std::to_string(_data._namesString.size())<<std::endl;

  std::cout<<"There are "<<std::to_string(_data.getNumBoolean())<<std::string(" boolean inputs, ")<<std::to_string(_data.getNumReal())<<std::string(" real inputs, ")<<std::to_string(_data.getNumInteger())<<std::string(" integer inputs and ")<<std::to_string(_data.getNumString())<<std::string(" string inputs.")<<std::endl;

  // The values for the inputs per type.
  _data._valuesReal = (fmi1_real_t*) calloc(_data.getNumReal(), sizeof(fmi1_real_t));
  _data._valuesInteger = (fmi1_integer_t*) calloc(_data.getNumInteger(), sizeof(fmi1_integer_t));
  _data._valuesBoolean = (fmi1_boolean_t*) calloc(_data.getNumBoolean(), sizeof(fmi1_boolean_t));
  _data._valuesString = (fmi1_string_t*) calloc(_data.getNumString(), sizeof(fmi1_string_t));
  // malloc attributes
  _data._attrReal = (AttributesReal*) calloc(_data.getNumReal(), sizeof(AttributesReal));

  // init keymap and attributes
  // ------------------
  _keyboardToKeyMap[119] = KEY_W;
  _keyboardToKeyMap[97] = KEY_A;
  _keyboardToKeyMap[115] = KEY_S;
  _keyboardToKeyMap[100] = KEY_D;

  int k = 0;
  // Make map from keys to input values.
  for (unsigned int r = 0; r < _data.getNumReal(); ++r)
  {
    _keyToInputMap[keys_real[r]] =
    {   fmi1_base_type_real, r};
    std::cout<<"Assign real input "<<std::to_string(r)<<std::string(" to key ")<<std::to_string(keys_real[r])<<std::endl;
    std::cout<<"min "<<std::to_string(_data._attrReal[r]._min)<<std::string(" max ")<<std::to_string(_data._attrReal[r]._max)<<std::endl;
    ++k;
  }
  for (unsigned int r = 0; r < _data.getNumInteger(); ++r)
  {
    _keyToInputMap[keys_real[k]] =
    {   fmi1_base_type_int, r};
    ++k;
  }
  for (unsigned int r = 0; r < _data.getNumBoolean(); ++r)
  {
    _keyToInputMap[keys_bool[r]] =
    {   fmi1_base_type_bool, r};
    std::cout<<"Assign boolean input "<<boolToString(r)<<std::string(" to key ")<<std::to_string(keys_bool[r])<<std::endl;
    ++k;
  }
  for (unsigned int r = 0; r < _data.getNumString(); ++r)
  {
    _keyToInputMap[keys_real[k]] =
    {   fmi1_base_type_str, r};
    ++k;
  }

  for (keyMapIter iter = _keyToInputMap.begin(); iter != _keyToInputMap.end(); ++iter)
    std::cout<<"Key: "<<std::to_string(iter->first)<<std::string(" --> Values: ")<<std::to_string(iter->second._baseType)<<std::string(" ")<<std::to_string(iter->second._valueIdx)<<std::endl;
}

void InputData::resetInputValues()
{
  // Reset real input values to 0.0.
  for (size_t r = 0; r < _data.getNumReal(); ++r)
    _data._valuesReal[r] = 0.0;
  // Reset integer input values to 0.
  for (size_t i = 0; i < _data.getNumInteger(); ++i)
    _data._valuesInteger[i] = 0;

  // Reset boolean and string values.
  resetDiscreteInputValues();
}

void InputData::resetDiscreteInputValues()
{
  // Reset boolean input values to false.
  for (unsigned int b = 0; b < _data.getNumBoolean(); ++b)
    _data._valuesBoolean[b] = false;
  // Reset string input values to empty string.
  for (unsigned int s = 0; s < _data.getNumString(); ++s)
    _data._valuesString[s] = "";
}

/*-----------------------------------------
 * GETTERS and SETTERS
 *---------------------------------------*/

/// \todo: What do we do with the variable status?
void InputData::setInputsInFMU(fmi1_import_t* fmu)
{
  fmi1_status_t status = fmi1_import_set_real(fmu, _data._vrReal, _data.getNumReal(), _data._valuesReal);
  status = fmi1_import_set_integer(fmu, _data._vrInteger, _data.getNumInteger(), _data._valuesInteger);
  status = fmi1_import_set_boolean(fmu, _data._vrBoolean, _data.getNumBoolean(), _data._valuesBoolean);
  status = fmi1_import_set_string(fmu, _data._vrBoolean, _data.getNumString(), _data._valuesString);
}

void InputData::getVariableNames(fmi1_import_variable_list_t* varLst, const int numVars, std::vector<std::string>& varNames)
{
  std::string name("");
  fmi1_import_variable_t* var = nullptr;
  const char* na = nullptr;
  for (int idx = 0; idx < numVars; ++idx)
  {
    var = fmi1_import_get_variable(varLst, idx);
    na = fmi1_import_get_variable_name(var);
    name.assign(na);
    varNames.push_back(name);
  }
}

fmi1_real_t* InputData::getRealValues() const
{
  return _data._valuesReal;
}

fmi1_integer_t* InputData::getIntValues() const
{
  return _data._valuesInteger;
}

fmi1_boolean_t* InputData::getBoolValues() const
{
  return _data._valuesBoolean;
}

bool InputData::setRealInputValueForInputKey(const inputKey key, const double value)
{
  keyMapIter iter = _keyToInputMap.find(key);

  if (iter != _keyToInputMap.end())
  {
    KeyMapValue iterValue = iter->second;
    int baseTypeIdx = iterValue._baseType;
    if (baseTypeIdx == fmi1_base_type_real)
    {
      int realIdx = iterValue._valueIdx;
      //value
      double min = _data._attrReal[realIdx]._min;
      double max = _data._attrReal[realIdx]._max;
      double val = value / 32767.0;
      _data._valuesReal[iterValue._valueIdx] = val;
      return true;
    }
    else
    {
      std::cout<<"The value is not for a real input."<<std::endl;
      return false;
    }
  }
  return false;
}

const keyboardMap* InputData::getKeyboardMap()
{
  return &_keyboardToKeyMap;
}

const keyMap* InputData::getKeyMap()
{
  return &_keyToInputMap;
}

const InputValues* InputData::getInputValues()
{
  return &_data;
}

/*-----------------------------------------
 * PRINTERS
 *---------------------------------------*/

void InputData::printValues() const
{
  for (unsigned int r = 0; r < _data.getNumReal(); ++r)
    std::cout << "realinput " << r << " (" << _data._namesReal.at(r) << ") " << " is " << _data._valuesReal[r] << std::endl;
  for (unsigned int i = 0; i < _data.getNumInteger(); ++i)
    std::cout << "integer input " << i << " (" << _data._namesInteger.at(i) << ") " << " is " << _data._valuesInteger[i] << std::endl;
  for (unsigned int b = 0; b < _data.getNumBoolean(); ++b)
    std::cout << "bool input " << b << " (" << _data._namesBool.at(b) << ") " << " is " << _data._valuesBoolean[b] << std::endl;
  for (unsigned int s = 0; s < _data.getNumString(); ++s)
    std::cout << "string input " << s << " (" << _data._namesString.at(s) << ") " << " is " << _data._valuesString[s] << std::endl;
}

void InputData::printKeyToInputMap() const
{
  std::cout << "KEY TO INPUT MAP" << std::endl;
  for (std::map<inputKey, KeyMapValue>::const_iterator it = _keyToInputMap.begin(); it != _keyToInputMap.end(); ++it)
    std::cout << it->first << " " << keyMapValueToString(it->second) << "\n";
}


/*-----------------------------------------
 * Free Functions
 *---------------------------------------*/

/// \todo: No return?!
inputKey getInputDataKeyForString(std::string keyString)
{
  if (!keyString.compare("JOY_1_X"))
    return JOY_1_X;
  else if (!keyString.compare("JOY_1_Y"))
    return JOY_1_Y;
  else if (!keyString.compare("JOY_2_X"))
    return JOY_2_X;
  else if (!keyString.compare("JOY_2_Y"))
    return JOY_2_Y;
  else if (!keyString.compare("KEY_W"))
    return KEY_W;
  else if (!keyString.compare("KEY_A"))
    return KEY_A;
  else if (!keyString.compare("KEY_S"))
    return KEY_S;
  else if (!keyString.compare("KEY_D"))
    return KEY_D;
  else
    std::cout << "getInputKeyForString  failed" << std::endl;
}

std::string keyMapValueToString(KeyMapValue val)
{
  return std::string(fmi1_base_type_to_string(val._baseType)).append(std::to_string(val._valueIdx));
}

int causalityEqual(fmi1_import_variable_t* var, void* enumIdx)
{
  // Get causality attribute, i.e., fmi1_causality_enu_input, fmi1_causality_enu_output, etc.
  fmi1_causality_enu_t causality = fmi1_import_get_causality(var);
  // Use static cast to "convert" void* to fmi1_causality_enu_t*. With that, we can compare the attributes.
  fmi1_causality_enu_t* toComp = static_cast<fmi1_causality_enu_t*>(enumIdx);

  //std::cout<<"causalityEqual: fmi1_causality_enu_t causality is ") + std::to_string(causality) + " and enumIdx has attribute " + std::to_string(*toComp) + std::string("."), Util::LC_INIT, Util::LL_INFO);
  if (*toComp == causality)
  {
    //std::cout<<"They are equal!"), Util::LC_INIT, Util::LL_INFO);
    //std::cout << "the input var: " << fmi1_import_get_variable_name(var) << std::endl;
    return 1;
  }
  else
  {
    //std::cout<<"They are not equal!"), Util::LC_INIT, Util::LL_INFO);
    return 0;
  }
}

int baseTypeEqual(fmi1_import_variable_t* var, void* refBaseType)
{
  fmi1_base_type_enu_t baseType = fmi1_import_get_variable_base_type(var);
  fmi1_base_type_enu_t* toComp = static_cast<fmi1_base_type_enu_t*>(refBaseType);

  //std::cout<<"baseTypeEqual: fmi1_base_type_enu_t baseType is ") + std::to_string(baseType) + " and refBaseType is of type" + std::to_string(*toComp) + std::string("."), Util::LC_INIT, Util::LL_INFO);
  if (*toComp == baseType)
  {
    //std::cout<<"They are equal!"), Util::LC_INIT, Util::LL_INFO);
    return 1;
  }
  else
  {
    //std::cout<<"They are not equal!"), Util::LC_INIT, Util::LL_INFO);
    return 0;
  }
}
