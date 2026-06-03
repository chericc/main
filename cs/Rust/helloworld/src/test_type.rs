
fn foo(x : u8) -> u32
{
    return x as u32 * x as u32;
}

pub fn test_type()
{
    crate::log!("Hello, world!");

    let x : i32 = 42;
    let y = 42;

    let c = foo(x as u8);
    let d = foo(y);
    crate::log!("c={}", c);
    crate::log!("d={}", d);
}