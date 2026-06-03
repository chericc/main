
enum Operation {
    Add(u64, u64),
    Subtract(u64, u64),
}

enum CalcResult {
    Ok(u64),
    Invalid(String),
}

fn calculate(op: Operation) -> CalcResult {
    match op {
        Operation::Add(a, b) => {
            CalcResult::Ok(a + b)
        },
        Operation::Subtract(a, b) => {
            if a >= b {
                CalcResult::Ok(a - b)
            } else {
                CalcResult::Invalid("Underflow".to_string())
            }
        }
    }
}

fn add_sub_example() {
    let op_add = Operation::Add(10, 20);
    let op_sub = Operation::Subtract(5, 10);
    let op_arr = [op_add, op_sub];
    for x in op_arr {
        match calculate(x) {
            CalcResult::Ok(result) => crate::log!("add: {}", result),
            CalcResult::Invalid(msg) => crate::log!("sub: {}", msg),
        }
    }
}

pub fn test()
{
    add_sub_example();
}