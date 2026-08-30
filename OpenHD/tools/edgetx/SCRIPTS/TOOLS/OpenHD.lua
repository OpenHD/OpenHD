-- OpenHD settings sender. Uses GV1..GV4 in flight mode 0 to communicate
-- with the OHDSET mixer script.
local items = {
  {"Frequency", 1, {2412,2437,2462,2484,5180,5200,5220,5240,5745,5765,5785,5805,5825}},
  {"Bandwidth", 2, {10,20,40}},
  {"MCS",       3, {0,1,2,3,4}},
  {"TX power %",4, {20,40,60,80,100}},
  {"FHSS",      6, {0,1}}
}
local row, choice, message = 1, 1, ""

local function wireValue(item, shown)
  if item[2] == 1 then
    if shown == 2484 then return 14 end
    if shown >= 2412 and shown <= 2472 then return (shown - 2407) / 5 end
    return shown / 5
  end
  if item[2] == 2 then return shown == 10 and 0 or (shown == 20 and 1 or 2) end
  return shown
end

local function send()
  local item = items[row]
  model.setGlobalVariable(0, 0, item[2])
  local value = wireValue(item, item[3][choice])
  model.setGlobalVariable(1, 0, value % 1024)
  model.setGlobalVariable(2, 0, math.floor(value / 1024))
  local token = model.getGlobalVariable(3, 0)
  model.setGlobalVariable(3, 0, token == 0 and 1 or 0)
  message = "Sent " .. item[1] .. " = " .. item[3][choice]
end

local function run(event)
  if event == EVT_VIRTUAL_NEXT then row = row % #items + 1; choice = 1
  elseif event == EVT_VIRTUAL_PREV then row = (row + #items - 2) % #items + 1; choice = 1
  elseif event == EVT_VIRTUAL_INC then choice = choice % #items[row][3] + 1
  elseif event == EVT_VIRTUAL_DEC then choice = (choice + #items[row][3] - 2) % #items[row][3] + 1
  elseif event == EVT_VIRTUAL_ENTER then send() end
  lcd.clear()
  lcd.drawText(2, 2, "OpenHD RC settings", MIDSIZE)
  for i,item in ipairs(items) do
    local flags = i == row and INVERS or 0
    local suffix = item[2] == 6 and (item[3][i==row and choice or 1] == 1 and "ON" or "OFF") or
                   tostring(item[3][i==row and choice or 1])
    lcd.drawText(4, 22 + (i-1)*12, item[1] .. ": " .. suffix, flags)
  end
  lcd.drawText(2, 88, "ENTER sends; +/- changes", SMLSIZE)
  lcd.drawText(2, 100, message, SMLSIZE)
  return 0
end
return {run=run}
