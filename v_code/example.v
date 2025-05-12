///
multiline comment
///
// single line comment

///
function definition:
<return_type> <function_name>:<?argument 1> | <?argument 2> ...(

);

function call:

///


null foo:{
    do_something[]
    return;
};

int bar:int x | int y{
    return x+y;
};

str f:int|int{
    return f:0 + f:1; //access arguments without names
}

print("hi");
.foo;
print(.bar(1,3));