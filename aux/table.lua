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
