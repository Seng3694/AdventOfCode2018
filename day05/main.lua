local function init_patterns(patterns)
    for i = 65, 90 do -- A - Z
        table.insert(patterns, string.char(i) .. string.char(i + 32))
        table.insert(patterns, string.char(i + 32) .. string.char(i))
    end
end

local function solve_part1(text, patterns)
    local length = #text
    repeat
        length = #text
        for _, pattern in ipairs(patterns) do
            text = string.gsub(text, pattern, "")
        end
    until #text == length

    return length
end

local function solve_part2(text, patterns)
    local minLength = math.maxinteger
    for i = 65, 90 do -- A - Z
        local clone = string.gsub(text, "[" .. string.char(i) .. string.char(i + 32) .. "]", "")
        local length = solve_part1(clone, patterns)
        if length < minLength then minLength = length end
    end
    return minLength
end

local function main()
    local file = io.open("day05/input.txt", "r")
    if not file then return end
    local text = file:read("a")
    text = string.sub(text, 1, #text - 1)

    local patterns = {}
    init_patterns(patterns)

    local part1 = solve_part1(text, patterns)
    local part2 = solve_part2(text, patterns)

    print(part1)
    print(part2)
end

main()
