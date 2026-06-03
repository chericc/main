
fn foo(x : u8) -> u32
{
    return x as u32 * x as u32;
}

pub fn test_type()
{
    println!("Hello, world!");

    let x : i32 = 42;
    let y = 42;

    let c = foo(x as u8);
    let d = foo(y);
    println!("c={}", c);
    println!("d={}", d);
}