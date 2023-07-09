local max = math.max
local min = math.min

local function parse(line)
    local x, y, w, h = string.match(line, "@ (%d+),(%d+): (%d+)x(%d+)")
    return {
        x = tonumber(x),
        y = tonumber(y),
        w = tonumber(w),
        h = tonumber(h),
    }
end

local function get_intersection_area(a, b)
    local interLeft = max(a.x, b.x)
    local interTop = max(a.y, b.y)
    local interRight = min(a.x + a.w, b.x + b.w)
    local interBottom = min(a.y + a.h, b.y + b.h)

    if interLeft < interRight and interTop < interBottom then
        return {
            x = interLeft,
            y = interTop,
            w = interRight - interLeft,
            h = interBottom - interTop,
        }
    end
end

local function hash_point(point)
    return (point.x * 31843249) ~ (point.y * 40519093)
end

local function solve_part1(claims)
    local intersections = {}
    for i = 1, #claims - 1 do
        for j = i + 1, #claims do
            local intersection = get_intersection_area(
                claims[i],
                claims[j])
            if intersection then
                table.insert(
                    intersections,
                    intersection)
            end
        end
    end

    local points = {}
    local hp = hash_point
    for _, v in ipairs(intersections) do
        for y = 1, v.h do
            for x = 1, v.w do
                local point = {
                    x = v.x + x,
                    y = v.y + y,
                }
                points[hp(point)] = true
            end
        end
    end

    local entries = 0
    for _ in pairs(points) do entries = entries + 1 end
    return entries
end

local function main()
    local file = io.open("day03/input.txt", "r")
    if not file then return end

    local claims = {}
    for line in file:lines() do
        table.insert(claims, parse(line))
    end
    file:close()

    local part1 = solve_part1(claims)

    print(part1)
end

main()
