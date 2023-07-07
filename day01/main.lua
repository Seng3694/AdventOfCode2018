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

local function main()
    local file = io.open("day01/input.txt", "r")
    if not file then return end

    local numbers = {}
    for line in file:lines() do
        table.insert(numbers, tonumber(line))
    end
    file:close()

    local part1 = solve_part1(numbers)

    print(part1)
end

main()
