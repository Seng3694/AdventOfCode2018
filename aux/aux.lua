local aux = {}

function aux.wrap(num, limit)
    if num > limit then
        return 1
    else
        return num
    end
end

return aux
