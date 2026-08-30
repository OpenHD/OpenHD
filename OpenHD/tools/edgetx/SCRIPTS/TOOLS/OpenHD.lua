-- toolName = OpenHD RC Settings
-- OpenHD settings sender. Uses GV1..GV4 in flight mode 0 to communicate
-- with the OHDSET mixer script.

local items = {
  {label="OHD channel", id=1, values={5700,5745,5785,5825,5865,5260,5280}, choice=1},
  {label="Bandwidth", id=2, values={10,20,40}, choice=1},
  {label="MCS",       id=3, values={0,1,2,3,4}, choice=1},
  {label="TX power",  id=4, values={20,40,60,80,100}, choice=1},
  {label="FHSS (Air)", id=6, values={0}, choice=1}
}

local selected, message, messageUntil, editing = 1, "", 0, false
local frequencyLabels = {
  [5700]="OHD 1  5700 MHz", [5745]="OHD 2  5745 MHz",
  [5785]="OHD 3  5785 MHz", [5825]="OHD 4  5825 MHz",
  [5865]="OHD 5  5865 MHz", [5260]="OHD 6  5260 MHz",
  [5280]="OHD 7  5280 MHz"
}
local cards = {
  {14,54,220,50}, {246,54,220,50},
  {14,112,220,50}, {246,112,220,50},
  {14,170,220,50}
}
local sendButton = {246,170,220,50}

local function wireValue(item, shown)
  if item.id == 1 then
    if shown == 2484 then return 14 end
    if shown >= 2412 and shown <= 2472 then return (shown - 2407) / 5 end
    return shown / 5
  end
  if item.id == 2 then return shown == 10 and 0 or (shown == 20 and 1 or 2) end
  return shown
end

local function shownValue(item)
  local value = item.values[item.choice]
  if item.id == 1 then return frequencyLabels[value] end
  if item.id == 2 then return value .. " MHz" end
  if item.id == 4 then return value .. "%" end
  if item.id == 6 then return "DISABLE" end
  return tostring(value)
end

local function changeChoice(delta)
  local item = items[selected]
  item.choice = (item.choice - 1 + delta) % #item.values + 1
end

local function send()
  local item = items[selected]
  local shown = item.values[item.choice]
  local value = wireValue(item, shown)
  model.setGlobalVariable(0, 0, item.id)
  model.setGlobalVariable(1, 0, value % 1024)
  model.setGlobalVariable(2, 0, math.floor(value / 1024))
  local token = model.getGlobalVariable(3, 0)
  model.setGlobalVariable(3, 0, token == 0 and 1 or 0)
  message = item.label .. "  " .. shownValue(item) .. " sent"
  messageUntil = getTime() + 250
  editing = false
end

local function inside(x, y, rect)
  return x >= rect[1] and x < rect[1] + rect[3] and
         y >= rect[2] and y < rect[2] + rect[4]
end

local function handleTouch(event, touchState)
  if event == EVT_TOUCH_TAP then
    local x, y = touchState.x, touchState.y
    if inside(x, y, sendButton) then send(); return end
    for i, rect in ipairs(cards) do
      if inside(x, y, rect) then
        selected = i
        editing = false
        if x < rect[1] + 52 then changeChoice(-1)
        elseif x >= rect[1] + rect[3] - 52 then changeChoice(1) end
        return
      end
    end
  elseif event == EVT_TOUCH_SLIDE then
    if touchState.swipeUp then selected = (selected + #items - 2) % #items + 1
    elseif touchState.swipeDown then selected = selected % #items + 1
    elseif touchState.swipeLeft then changeChoice(-1)
    elseif touchState.swipeRight then changeChoice(1) end
  end
end

local function handleKeys(event)
  if event == EVT_VIRTUAL_NEXT then
    if editing then changeChoice(1) else selected = selected % #items + 1 end
  elseif event == EVT_VIRTUAL_PREV then
    if editing then changeChoice(-1) else selected = (selected + #items - 2) % #items + 1 end
  elseif event == EVT_VIRTUAL_INC then changeChoice(1)
  elseif event == EVT_VIRTUAL_DEC then changeChoice(-1)
  elseif event == EVT_VIRTUAL_ENTER then
    if editing then send() else editing = true end
  elseif event == EVT_VIRTUAL_EXIT then
    if editing then editing = false else return 2 end
  end
  return 0
end

local function drawCard(i, colors)
  local rect, item = cards[i], items[i]
  local active = i == selected
  local fill = active and colors.selected or colors.card
  local border = active and colors.accent or colors.border
  lcd.drawFilledRectangle(rect[1], rect[2], rect[3], rect[4], fill)
  lcd.drawRectangle(rect[1], rect[2], rect[3], rect[4], border, active and 3 or 1)
  lcd.drawText(rect[1] + 12, rect[2] + 5, string.upper(item.label), SMLSIZE + colors.muted)
  lcd.drawText(rect[1] + rect[3] / 2, rect[2] + 22, shownValue(item), MIDSIZE + CENTER + colors.text)
  if active then
    lcd.drawText(rect[1] + 17, rect[2] + 20, "<", MIDSIZE + colors.accent)
    lcd.drawText(rect[1] + rect[3] - 17, rect[2] + 20, ">", MIDSIZE + RIGHT + colors.accent)
    if editing then lcd.drawText(rect[1] + rect[3] - 10, rect[2] + 5, "EDIT", SMLSIZE + RIGHT + colors.accent) end
  end
end

local function draw()
  local colors = {
    bg=lcd.RGB(12,18,27), card=lcd.RGB(25,34,46),
    selected=lcd.RGB(35,48,62), border=lcd.RGB(55,69,84),
    accent=lcd.RGB(255,139,31), text=lcd.RGB(239,244,248),
    muted=lcd.RGB(145,160,175), success=lcd.RGB(35,166,108),
    dark=lcd.RGB(16,22,30)
  }
  lcd.clear(colors.bg)
  lcd.drawFilledRectangle(0, 0, LCD_W, 42, colors.dark)
  lcd.drawFilledRectangle(0, 40, LCD_W, 2, colors.accent)
  lcd.drawText(14, 6, "OPENHD", DBLSIZE + colors.text)
  lcd.drawText(LCD_W - 14, 11, "RC LINK SETTINGS", SMLSIZE + RIGHT + colors.muted)
  for i = 1, #items do drawCard(i, colors) end

  local recent = message ~= "" and getTime() < messageUntil
  local buttonColor = recent and colors.success or colors.accent
  lcd.drawFilledRectangle(sendButton[1], sendButton[2], sendButton[3], sendButton[4], buttonColor)
  lcd.drawText(sendButton[1] + sendButton[3] / 2, sendButton[2] + 8,
               recent and "SENT" or "SEND TO OPENHD", MIDSIZE + CENTER + colors.dark)
  lcd.drawText(sendButton[1] + sendButton[3] / 2, sendButton[2] + 31,
               recent and message or "Tap or press ENTER", SMLSIZE + CENTER + colors.dark)

  lcd.drawText(14, 238, editing and "ROTARY  Change" or "ROTARY  Select", SMLSIZE + colors.muted)
  lcd.drawText(LCD_W / 2, 238, editing and "ENTER  Send" or "ENTER  Edit", SMLSIZE + CENTER + colors.muted)
  lcd.drawText(LCD_W - 14, 238, "EXIT  RTN", SMLSIZE + RIGHT + colors.muted)
  lcd.drawText(LCD_W / 2, 255, "Touch cards:  <  value  >", SMLSIZE + CENTER + colors.muted)
end

local function run(event, touchState)
  local exit = 0
  if touchState then handleTouch(event, touchState)
  else exit = handleKeys(event) end
  draw()
  return exit
end

return {run=run}
