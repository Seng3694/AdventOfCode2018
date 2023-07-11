local function parse(path)
    local file = io.open(path, "r")
    if not file then return end

    local root = nil
    local current = root
    local length = 0
    repeat
        local c = file:read(1)
        if c and c ~= '\n' then
            local node = { value = string.byte(c) }
            if current == nil then
                root = node
                current = node
            else
                node.left = current
                current.right = node
                current = node
            end
            length = length + 1
        end
    until not c

    file:close()
    return root, length
end

local function are_compatible(a, b)
    return math.abs(a - b) == 32
end

local function solve_part1(root, length)
    local changed = false
    local current = root
    local next = current.right
    repeat
        changed = false
        while current.right ~= nil do
            next = current.right
            if are_compatible(current.value, next.value) then
                local leftOfCurrent = current.left
                local rightOfNext = next.right
                if leftOfCurrent == nil then
                    root = rightOfNext
                    rightOfNext.left = nil
                elseif rightOfNext == nil then
                    leftOfCurrent.right = nil
                else
                    leftOfCurrent.right = rightOfNext
                    rightOfNext.left = leftOfCurrent
                end
                current.left = nil
                current.right = nil
                next.left = nil
                next.right = nil
                changed = true
                length = length - 2
            end
            current = next
        end
        current = root
    until not changed

    return length
end

local function solve_part2(list, length)
    for i = 65, 90 do -- A - Z

    end
end

local function main()
    local list, length = parse("day05/input.txt")
    local part1 = solve_part1(list, length)

    print(part1)
end

main()
