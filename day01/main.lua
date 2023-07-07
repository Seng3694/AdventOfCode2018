require "aux.table"
local aux = require "aux.aux"

local function solve_part1(numbers)
    return table.reduce(
        numbers,
        function(acc, value)
            return acc + value
        end
    )
end

local function solve_part2(numbers)
    local frequencies = { [0] = 1 }
    local index = 1
    local current = 0
    while true do
        current = current + numbers[index]
        index = aux.wrap(index + 1, #numbers)
        if frequencies[current] then
            return current
        else
            frequencies[current] = 1
        end
    end
end

local function main()
    local file = io.open("day01/input.txt", "r")
    if not file then return end

    local numbers = {}
    for line in file:lines() do
        table.insert(numbers, tonumber(line))
    end
    file:close()

    local part1 = solve_part1(numbers)
    local part2 = solve_part2(numbers)

    print(part1)
    print(part2)
end

main()
