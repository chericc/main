
struct Point {
    x: u32,
    y: u32
}

impl Point {
    fn new(x: u32, y: u32) -> Self {
        Point {x, y}
    }
    fn increment_x(&mut self) {
        self.x += 1
    }
    fn add(&mut self, other: &Point) {
        self.x += other.x;
        self.y += other.y;
    }
    fn transform(&mut self) -> Point {
        Point { x: self.x * self.x, y: self.y * self.y }
    }
}

pub fn test() {
    let mut p = Point::new(10, 20);
    crate::log!("p: x={},y={}", p.x, p.y);
    p.increment_x();
    crate::log!("p: x={},y={}", p.x, p.y);
    let p1 = Point::new(20, 20);
    p.add(&p1);
    crate::log!("p: x={},y={}", p.x, p.y);
    p = p.transform();
    crate::log!("p: x={},y={}", p.x, p.y);
}