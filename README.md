# Keylogger
Capture keyboard input, and then send it to a server and the server saves it to file.

# How project works 
- EventHandler가 Event(key 입력 들어옴)가 발생하면 API를 사용해 window 화면과 내가 만든 secondary Program가 정보를 받음.
- 이 프로그램은 프론트 화면에서 보이지 않으며, 기기 전원이 켜지면 별다른 명령 없이 자동으로 동작함.
- 클라이언트가 수집한 데이터를 특정 서버로 전송함.(TCP 통신 사용)
- Linux에 존재하는 서버는 client에게 받은 정보를 client의 IP를 이름으로 하여 Ip_files라는 디렉토리에 파일을 만들고, 받은 내용을 저장함.
- 여러 client와 동시에 연결 가능함.
- 키보드로부터 입력이 있으면 client가 server와 연결 시도.(3초마다 입력이 있는지 검사함.)
- client 프로그램 종료 방법: ctrl+Q

# 실행 방법
- server 프로그램을 먼저 실행하고(Linux에서 background로 돌려두면 편함.) client 서버를 그 다음에 실행할 것.
- client 프로그램은 프로젝트 속성에서 링커-시스템-하위 시스템을 창(/SUBSYSTEM:WINDOWS)으로 바꾸어야 실행 화면에 뜨지 않음. (! main()도 콘솔형과 다르게 int WINAPI WinMain 사용.)
  ![스크린샷 2023-11-09 234053](https://github.com/goodrain95/Keylogger_with_TCP/assets/143669574/82dc6301-7e2c-4de4-9692-2fcce1d22016)
- server 프로그램은 추가 설정 없이 컴파일하면 됨.
## 컴퓨터 부팅과 함께 실행되게 하는 방법
- 실행 앱 -> shell:startup 열고 이곳에 client 실행 프로그램 넣기.
- 정말 실행되는지 확인하고 싶다면 작업관리자->시작 프로그램 확인해보기
## IDE
### OS
- server: Linux / client: Windows 11
### program
- server: vim / client: VISUAL STUDIO 2022
### Language Package: C, C++
### how to compile
- server: type './keylogger_tcp_server' / client: push Ctrl + F5
