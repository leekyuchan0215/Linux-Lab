use std::io;

// 사용자로부터 한 줄을 읽어 숫자로 파싱하는 헬퍼 함수
fn read_line_to_string() -> String {
    let mut input = String::new();
    io::stdin().read_line(&mut input).expect("입력 읽기 실패");
    input.trim().to_string()
}

// 사용자로부터 행렬의 원소들을 입력받아 Vec<i32>로 반환하는 함수
fn read_matrix_elements(size: usize) -> Vec<i32> {
    // 필요한 크기만큼의 공간을 힙에 동적으로 예약합니다. (Vec<T> 활용)
    let mut matrix = Vec::with_capacity(size); 
    
    println!("원소를 입력해주세요 (총 {}개):", size);

    while matrix.len() < size {
        let input = read_line_to_string();
        
        for num_str in input.split_whitespace() {
            if let Ok(num) = num_str.parse::<i32>() {
                matrix.push(num); // Vec<i32>에 동적 추가
                if matrix.len() == size {
                    break;
                }
            } else {
                eprintln!("잘못된 입력입니다: '{}'. 숫자를 입력해주세요.", num_str);
            }
        }
    }
    matrix
}

fn main() {
    // 1. 첫 번째 행렬의 크기 입력
    println!("첫 번째 행렬의 행과 열 크기를 입력하세요:");
    let size1_input = read_line_to_string();
    let size1: Vec<usize> = size1_input
        .split_whitespace()
        .filter_map(|s| s.parse().ok())
        .collect();

    if size1.len() != 2 {
        eprintln!("올바른 행과 열 크기 2개를 입력해주세요.");
        return;
    }
    let (r1, c1) = (size1[0], size1[1]);
    let total_size1 = r1 * c1;

    // 2. 두 번째 행렬의 크기 입력
    println!("두 번째 행렬의 행과 열 크기를 입력하세요:");
    let size2_input = read_line_to_string();
    let size2: Vec<usize> = size2_input
        .split_whitespace()
        .filter_map(|s| s.parse().ok())
        .collect();

    if size2.len() != 2 {
        eprintln!("올바른 행과 열 크기 2개를 입력해주세요.");
        return;
    }
    let (r2, c2) = (size2[0], size2[1]);
    let total_size2 = r2 * c2;

    // 3. 덧셈 가능 여부 확인
    if r1 != r2 || c1 != c2 {
        println!("행렬 크기가 달라서 덧셈을 할 수 없습니다. ({}x{} != {}x{})", r1, c1, r2, c2);
        return;
    }
    let rows = r1;
    let cols = c1;

    // 4. 행렬 A, B 원소 입력 (Vec<i32>에 동적 저장)
    println!("--- 첫 번째 행렬 원소 입력 ---");
    let matrix_a = read_matrix_elements(total_size1);

    println!("--- 두 번째 행렬 원소 입력 ---");
    let matrix_b = read_matrix_elements(total_size2);

    // 5. 행렬 덧셈
    let mut result_matrix: Vec<i32> = Vec::new();
    for i in 0..total_size1 {
        // 행렬 덧셈 로직
        let sum = matrix_a[i] + matrix_b[i];
        result_matrix.push(sum); // 결과 벡터에 저장
    }
    
    // 6. 결과 출력
    println!("\n결과 행렬:");
    for r in 0..rows {
        for c in 0..cols {
            // 1차원 인덱스 계산 (r * cols + c)를 사용하여 2차원 형식으로 출력
            print!("{} ", result_matrix[r * cols + c]);
        }
        println!();
    }
}
