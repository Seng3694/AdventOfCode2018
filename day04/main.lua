local function parse(line)
    -- [1518-11-22 00:00] Guard #1231 begins shift
    local year, month, day, hour, minute, rest = string.match(line, "(%d+)-(%d+)-(%d+) (%d+):(%d+)] (.+)")
    return {
        year = tonumber(year),
        month = tonumber(month),
        day = tonumber(day),
        hour = tonumber(hour),
        minute = tonumber(minute),
        rest = rest
    }
end

local function compare_shift_data(a, b)
    if a.year ~= b.year then
        return b.year - a.year > 0
    end
    if a.month ~= b.month then
        return b.month - a.month > 0
    end
    if a.day ~= b.day then
        return b.day - a.day > 0
    end
    if a.hour ~= b.hour then
        return b.hour - a.hour > 0
    end
    return b.minute - a.minute > 0
end

local function parse_guard_schedules(data)
    table.sort(data, compare_shift_data)

    local guards = {}
    local i = 1
    while i <= #data do
        local guardId = string.match(data[i].rest, "(%d+)")
        local guard = guards[guardId] or { id = guardId, asleepRanges = {} }

        i = i + 1
        local sleepStartTime = 0
        local asleepRanges = {}
        --while it doesn't start with "Guard"
        --always starts with "falls asleep"
        while i <= #data and string.sub(data[i].rest, 1, 1) ~= 'G' do
            -- f = falls asleep
            if string.sub(data[i].rest, 1, 1) == 'f' then
                sleepStartTime = data[i].minute
            else -- wakes up
                table.insert(asleepRanges, {
                    from = sleepStartTime,
                    to = data[i].minute
                })
            end
            i = i + 1
        end
        --always ends with "wakes up"
        table.insert(guard.asleepRanges, asleepRanges)

        guards[guardId] = guard
    end

    return guards
end

local function solve_part1(data)
    local guardSchedules = parse_guard_schedules(data)

    --find guard with longest time asleep
    local totalSleepTimes = {}
    for guardId, guard in pairs(guardSchedules) do
        local sleepTime = 0
        for _, ranges in ipairs(guard.asleepRanges) do
            for _, range in ipairs(ranges) do
                sleepTime = sleepTime + (range.to - range.from)
            end
        end
        table.insert(totalSleepTimes, {
            id = guardId,
            time = sleepTime,
        })
    end
    table.sort(totalSleepTimes, function(a, b) return a.time > b.time end)

    local first = totalSleepTimes[1]

    local minutes = {}
    for i = 1, 60 do
        table.insert(minutes, { minute = i - 1, count = 0 })
    end

    for _, ranges in ipairs(guardSchedules[first.id].asleepRanges) do
        for _, range in ipairs(ranges) do
            for i = range.from, range.to - 1 do
                minutes[i + 1].count = minutes[i + 1].count + 1
            end
        end
    end

    table.sort(minutes, function(a, b) return a.count > b.count end)

    return first.id * minutes[1].minute
end

local function main()
    local file = io.open("day04/input.txt", "r")
    if not file then return end

    local data = {}
    for line in file:lines() do
        table.insert(data, parse(line))
    end
    file:close()

    local part1 = solve_part1(data)

    print(part1)
end

main()
