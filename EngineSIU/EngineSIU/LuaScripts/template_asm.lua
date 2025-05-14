States = {
    { Name = "Entry", Sequence = "" },
}

Transitions = {
    {
        From = "Entry",
        To = "Input first state name",
        Condition = function()
            return true
        end
    }
}