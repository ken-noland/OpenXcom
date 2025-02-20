-- Function to recursively print table contents
function printTable(t, name, indent)
    if not indent then indent = 0 end
    if not name then name = "base" end

    local formatting = string.rep("  ", indent) .. name .. ": "
    print(formatting .. "{")

    for k, v in pairs(t) do
        local keyType = type(k)
        local valueType = type(v)
        local keyName = tostring(k)

        if valueType == "table" then
            print(string.rep("  ", indent + 1) .. keyName .. " (key type: " .. keyType .. ", value type: " .. valueType .. "):")
            printTable(v, keyName, indent + 2)
        else
            print(string.rep("  ", indent + 1) .. keyName .. " (key type: " .. keyType .. ", value type: " .. valueType .. "): " .. tostring(v))
        end
    end

    print(string.rep("  ", indent) .. "}")
end


print("begin")
printTable(game, "game", 0)
print("end")