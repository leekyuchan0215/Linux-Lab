use std::collections::HashMap;
use std::io::{self, Write}; // 입출력 처리를 위한 모듈

fn main() {
    // 1. HashMap 생성
    // key: 이름(String), value: 전화번호(String)
    let mut phonebook: HashMap<String, String> = HashMap::new();

    loop {
        // 메뉴 출력
        println!("\n=== 전화번호부 (LAB/LAB2/Q3) ===");
        println!("1. 전화번호 추가");
        println!("2. 전화번호 검색");
        println!("3. 전체 목록");
        println!("4. 종료");
        print!("선택>> ");
        io::stdout().flush().unwrap(); // 화면에 '선택>>'이 바로 보이게 함

        // 사용자 입력 받기
        let mut choice = String::new();
        io::stdin().read_line(&mut choice).expect("입력 에러");
        let choice = choice.trim(); // 엔터키 제거

        match choice {
            "1" => {
                // --- [추가 기능] ---
                print!("이름 입력: ");
                io::stdout().flush().unwrap();
                let mut name = String::new();
                io::stdin().read_line(&mut name).unwrap();
                let name = name.trim().to_string();

                print!("전화번호 입력: ");
                io::stdout().flush().unwrap();
                let mut number = String::new();
                io::stdin().read_line(&mut number).unwrap();
                let number = number.trim().to_string();

                // HashMap에 데이터 삽입 (insert)
                // 만약 이미 있는 이름이면 덮어씌워집니다.
                phonebook.insert(name, number);
                println!("-> 저장되었습니다.");
            }
            "2" => {
                // --- [검색 기능] ---
                print!("검색할 이름: ");
                io::stdout().flush().unwrap();
                let mut search_name = String::new();
                io::stdin().read_line(&mut search_name).unwrap();
                let search_name = search_name.trim();

                // HashMap에서 데이터 찾기 (get)
                // get은 Option 타입을 반환하므로 match로 처리
                match phonebook.get(search_name) {
                    Some(number) => println!("-> {}의 번호: {}", search_name, number),
                    None => println!("-> '{}'님은 등록되지 않았습니다.", search_name),
                }
            }
            "3" => {
                // --- [전체 목록] ---
                println!("--- 저장된 목록 ({}) ---", phonebook.len());
                for (name, number) in &phonebook {
                    println!("이름: {}, 번호: {}", name, number);
                }
            }
            "4" => {
                println!("프로그램을 종료합니다.");
                break;
            }
            _ => {
                println!("잘못된 입력입니다. 1~4번 중 선택하세요.");
            }
        }
    }
}
