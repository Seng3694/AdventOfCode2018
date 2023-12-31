local lookup = {}

local function clear_table(t)
    for k in pairs(t) do
        t[k] = nil
    end
end

local function count_letter_occurences(id)
    local lk = lookup
    clear_table(lk)

    for i = 1, #id do
        local letter = id:sub(i, i)
        local count = lk[letter]
        if count == nil then
            lk[letter] = 1
        else
            lk[letter] = count + 1
        end
    end

    return lk
end

local function solve_part1(ids)
    local twos = 0
    local threes = 0
    for _, id in ipairs(ids) do
        local occurences = count_letter_occurences(id)
        local has2 = false
        local has3 = false
        for _, v in pairs(occurences) do
            has2 = has2 or v == 2
            has3 = has3 or v == 3
        end
        if has2 then twos = twos + 1 end
        if has3 then threes = threes + 1 end
    end
    return twos * threes
end

local function count_same_letters(a, b)
    -- assumes that a and b have the same length
    local count = 0
    local lastDifference = 0
    for i = 1, #a do
        if a:sub(i, i) == b:sub(i, i) then
            count = count + 1
        else
            lastDifference = i
        end
    end
    return count, lastDifference
end

local function solve_part2(ids)
    for i = 1, #ids - 1 do
        local a = ids[i]
        for j = i, #ids do
            local b = ids[j]
            local sameLetterCount, lastDifference = count_same_letters(a, b)
            local difference = #a - sameLetterCount
            if difference == 1 then
                return a:sub(1, lastDifference - 1) .. a:sub(lastDifference + 1)
            end
        end
    end
end

local function main()
    local file = io.open("day02/input.txt", "r")
    if not file then return end

    local ids = {}
    for line in file:lines() do
        table.insert(ids, line)
    end
    file:close()

    local part1 = solve_part1(ids)
    local part2 = solve_part2(ids)

    print(part1)
    print(part2)
end

main()
