function v = SENSOR_FT_OFFSET_TORQUE_Y()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = iDynTreeMEX(0, 49);
  end
  v = vInitialized;
end