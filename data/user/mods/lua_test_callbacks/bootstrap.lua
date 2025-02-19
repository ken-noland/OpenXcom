-- Function to recursively print table contents
function printTable(t, indent)
    if not indent then indent = 0 end
	
    for k, v in pairs(t) do
        local keyType = type(k)
        local valueType = type(v)
        local keyName = tostring(k)

        if valueType == "table" then
            print(string.rep("  ", indent + 1) .. keyName .. " (key type: " .. keyType .. ", value type: " .. valueType .. "): {")
            printTable(v, indent + 2)
        else
            print(string.rep("  ", indent + 1) .. keyName .. " (key type: " .. keyType .. ", value type: " .. valueType .. "): " .. tostring(v))
        end
    end
end


function onLoad(doc)
	-- print("Hello world")
	printTable(game)
	printTable(ui)
end


function onTest()
	print("testing ui")
	printTable(ui)
end

game.on_load_game:register_callback(onLoad)
--print("are we getting here")
ui.on_test_event:register_callback(onTest)
