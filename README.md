1. getNumFreePages 시스템 콜 추가
// in kalloc.c
- uint num_free_pages 전역변수 선언
- kinit1()에서 num_free_pages = 0 초기화
- kalloc()에서 num_free_pages-- : page 할당되었으므로 --
- kfree()에서 num_free_pages++ : page 회수되었으므로 ++
- getNumFreePages() 시스템 콜 만들기 : kalloc.c에 getNumFreePages() 함수 구현하고, usys.h, user.S, syscall.h, syscall.c, sysfile.c, defs.h에 시스템콜에 필요한 코드를 작성

2. 각 메모리 페이지들을 대상으로 reference counter 적용(물리 메모리 주소 사용)
- kalloc.c에 uint pgrefcount[PHYSTOP>>PGSHIFT] 전역변수 선언
- mmu.h에 #define PGSHIFT 12; 선언
  * page 크기가 4KB이므로 page 주소 저장하는데 12bit가 필요하다. 따라서 주소의 나머지 20bit는 page 번호에 해당하게 된다. page 번호로 page를 식별하기 때문에 PGSHIGT만큼 shift 연산을 한 것인다. 
  * 또한, physical memory가 들어갈 수 있는 최대는 PHYSTOP이므로(위는 devices 공간) 배열크기를 위와 같이 설정해준 것이다.
- get_refcount, inc_refcount, dec_refcount 함수 구현
  * reference counter 값을 가져오고, 증가시켜주고, 감소시켜주는 함수
  * kmem 내부가 아니라 전역변수로 선언했으므로 kmem을 lock 시키고 코드를 실행해주면 안됨
// kalloc.c
- freerange()에서 pgrefcount 배열 0으로 초기화
  * freerange에서 kfree(p)를 하기 전에 pgrefcount[주소] = 0; 코드를 넣어주어 physical memory 초기화시켜줌
- kfree()에서 dec_refcount() 호출
  * reference counter 값을 보고 새 메모리를 만들어줄 것인지 reference counter 값만 감소시켜줄 것인지 결정
  * freerange()에서 range만큼 kfree를 호출해 reference counter==0이면 num_free_pages++을 해주므로 initial때 num_free_pages 개수가 가능한 memory만큼 설정되게 됨
- vm.c의 copyuvm()에 inc_refcount 함수 호출
 * fork()시 copyuvm() 호출을 통해 child process는 parent process의 memory를 복사하게 된다.
 * Copy-on-Write은 수정 전까지 parent process와 child process가 같은 메모리를 참조한다. 따라서 child process가 생겨 메모리를 복사해주던 기존 코드 대신에 inc_refcount(pa)를 통해 page의 reference counter 값을 증가시켜줘야 한다.

3. copyuvm() 수정 in vm.c
- 위 함수 내의 memory pages를 생성하고 복사하는 부분을 기존 physical memory를 참조하도록 변경해줘야한다. 
  * 따라서 memory 공간을 새로 할당하고(mem=kalloc()) parent memory를 복사(memmove)하는 코드를 지워야한다.
  * 또한, 기존 physical memory를 사용하는 것이므로 physical memory 공간을 새롭게 free해주는 kfree(mem) 코드를 지워야한다.
- 기존 physical memory의 wirte 권한 없애야 함
  * page table entry에 해당하는 pte에 다음과 같은 연산을 하여 write 권한을 없앰. pte &= (~PTE_W)
  * 이렇게 만들어진 pte로 pte_address와 pte flag를 가져와 mappage 함수에서 page table을 새로 만들어준다.
  * 위의 작업이 끝난 후 lcr3함수를 호출하여 pgdir의 TLB를 초기화해준다.

4. page fault handler 구현
- trap.c에 있는 trap() 함수에 page fault 발생시 pagefault()로 가도록 하는 코드를 switch문 안에 넣어준다.
- vm.c에 pagefualt() 함수를 구현한다
  * 먼저 rc2() 함수를 호출해 page fault가 발생한 virtual address를 가져온다.
  * walkpgdir() 함수와 va를 사용하여 해당 page table entry를 확인하여 physical address(pa)를 가져온다.
  * pa의 reference counter 값(rc)를 확인하여 rc==1이면 write 권한을 추가해주고, rc >1이면 physical memory 새로운 공간에 기존 page를 복사해준다.

5. test application testcow.c 생성
- Makefile의 UPROGS 변수에 _testcow\를 추가해주고, EXTRA=\에 testcow.c를 추가해줍니다. 
