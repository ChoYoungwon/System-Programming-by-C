## 프로젝트 구성
### 1. Custom Shell (`custom_shell/`)
리눅스 기본 쉘의 핵심 기능을 구현한 사용자 정의 쉘 프로그램

**주요 기능:**
- 디렉토리 관리: `cd`, `mkdir`, `rmdir`, `ls`
- 파일 관리: `cat`, `cp`, `rm`, `rename`, `chmod`
- 링크 관리: `ln` (하드링크 및 심볼릭 링크)
- 프로세스 관리: `ps`, `kill`
- 프로그램 실행: 외부 프로그램 실행 지원

**주요 사항:**
- `/tmp/test` 베이스 경로로 제한된 안전한 실행 환경
- 상위 디렉토리 접근 제한으로 보안 강화

### 2. GTK File Manager (`dir_manage_gtk/`)
GTK4 기반의 GUI 파일 관리자 구현

**주요 기능:**
- **GUI 파일 브라우저**: 아이콘, 크기, 수정시간 표시
- **파일 관리**: 생성, 삭제, 이동, 복사, 이름변경
- **권한 관리**: chmod를 통한 파일 권한 변경
- **즐겨찾기**: 자주 사용하는 파일/폴더 바로가기
- **실시간 업데이트**: 파일 시스템 변경사항 즉시 반영

**기술적 특징:**
- **프론트엔드-백엔드 분리**: GTK4 UI + C 백엔드 프로세스
- **IPC 통신**: 메시지 큐를 통한 프로세스 간 통신 방식
- **비동기 처리**: 파일 작업 중 UI 응답성 유지 구현

## 빌드 방법

### Custom Shell
```bash
cd custom_shell
make
./program
```

### GTK File Manager
_build 디렉토리에 실행파일 컴파일 해두었습니다.
- 바로 실행 : `_build/column`

** 주의 사항**
- 프론트 실행 파일 : column
- 백엔드 실행 파일 : a.out
- fork를 사용해 백엔드는 자동으로 켜지도록 설정하였습니다.
- _build 디렉토리외 폴더로 setup할 경우 fork시 실행하는 경로가 달라져 오류 발생

#### 참고
컴파일 방법(_build 디렉토리 삭제 후)
```bash
$ sudo apt install libgtk-4-dev (GTK 4 개발 파일 패키지 설치)
$ meson setup _build
$ ninja -C _build
$ _build/column
```
![image](https://github.com/user-attachments/assets/c91ab84e-d4b3-4e00-8e20-67db94615e4c)



