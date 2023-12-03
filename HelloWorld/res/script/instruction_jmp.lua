-- ::HAHA::
-- goto HAHA

-- 处理闭合 upvalue 的情况
local x, y, z
do
    local a, b, c
    function foo()
        b = 1
    end
end
