table.reduce = function(list, fn, init)
    local acc = init
    for k, v in ipairs(list) do
        if k == 1 and not init then
            acc = v
        else
            acc = fn(acc, v)
        end
    end
    return acc
end

table.map = function(list, fn)
    local output = {}
    for i = 1, #list do
        output[i] = fn(list[i])
    end
    return output
end

table.filter = function(list, pred)
    local i = 0
    return function()
        i = i + 1
        while list[i] ~= nil and not pred(list[i]) do
            i = i + 1
        end
        return list[i]
    end
end
