#[macro_export]
macro_rules! log {
    ($($arg:tt)*) => {{
        fn _log_fn_() {}
        let _name = {
            let s = std::any::type_name_of_val(&_log_fn_);
            match s.rfind("::") {
                Some(pos) => &s[..pos],
                None => s,
            }
        };
        println!("[{}:{}] [{}] {}", file!(), line!(), _name, format!($($arg)*))
    }};
}

mod test_type;
mod test_enum_1;
mod test_enum_2;

fn main() {
    test_type::test_type();
    test_enum_1::test();
    test_enum_2::test();
}
