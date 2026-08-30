-- OpenHD four-channel settings transport.
-- Assign the four script outputs D2,D1,D0,CLK to four consecutive channels.
-- GV1=id, GV2=value low 10 bits, GV3=value high bit, GV4=send token.

local seq, lastToken, symbols, pos, ticks, clock = 0, -1, nil, 0, 0, -1024

local function xor5(a, b)
  local result, place = 0, 1
  for _ = 1, 5 do
    if (a % 2) ~= (b % 2) then result = result + place end
    a, b, place = math.floor(a / 2), math.floor(b / 2), place * 2
  end
  return result
end

local function crc5(data)
  local crc = 31
  for bit = 18, 0, -1 do
    local input = math.floor(data / 2^bit) % 2
    local feedback = (math.floor(crc / 16) % 2 + input) % 2
    crc = (crc * 2) % 32
    if feedback == 1 then crc = xor5(crc, 5) end
  end
  return xor5(crc, 31)
end

local function beginFrame(id, value)
  seq = (seq + 1) % 8
  local payload = id * 16384 + value * 8 + seq
  local frame = payload * 32 + crc5(payload)
  symbols = {7}
  for n = 7, 0, -1 do
    symbols[#symbols + 1] = math.floor(frame / 2^(n * 3)) % 8
  end
  pos, ticks = 1, 0
end

local function run()
  local token = model.getGlobalVariable(3, 0)
  if not symbols and token ~= lastToken then
    lastToken = token
    local id = model.getGlobalVariable(0, 0)
    local value = model.getGlobalVariable(1, 0) +
                  model.getGlobalVariable(2, 0) * 1024
    if id > 0 and id <= 31 and value >= 0 and value <= 2047 then
      beginFrame(id, value)
    end
  end
  local symbol = 0
  if symbols then
    symbol = symbols[pos]
    ticks = ticks + 1
    if ticks >= 15 then -- about 150ms on the normal 100Hz mixer scheduler
      ticks = 0
      clock = -clock
      pos = pos + 1
      if pos > #symbols then symbols = nil end
    end
  end
  local function level(bit) return math.floor(symbol / 2^bit) % 2 == 1 and 1024 or -1024 end
  return level(2), level(1), level(0), clock
end

return {run=run, output={"D2","D1","D0","CLK"}}
