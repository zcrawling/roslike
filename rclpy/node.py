from serializer import Serializer, Deserializer
from datatype import Datatype, _TypedValue
import zmq, threading, time
from concurrent.futures import ThreadPoolExecutor


class Node:
    def __init__(self, name, max_workers = 4):
        self.node_name = name
        self.context = zmq.Context()

        self.pub_sockets = {}
        self.sub_sockets = {}
        self.callbacks = {}
        self.executor = ThreadPoolExecutor(max_workers)

        self.running = True
        self.receiver_thread = threading.Thread(target=self._run_receiver, daemon=True)
        self.receiver_thread.start()

    def publish(self, topic: str, *args: _TypedValue):
        if topic not in self.pub_sockets:
            sock = self.context.socket(zmq.PUB)
            ipc_addr = f"ipc:///tmp/roslike_{topic}"
            sock.bind(ipc_addr)
            self.pub_sockets[topic] = sock
            print(f"[{self.node_name}] pub bound to {ipc_addr}")

        se = Serializer()
        for arg in args:
            se.pack(arg)

        self.pub_sockets[topic].send(bytes(se.buffer))

    def subscribe(self, topic: str, callback):
        if topic not in self.sub_sockets:
            sock = self.context.socket(zmq.SUB)
            sock.setsockopt(zmq.SUBSCRIBE, b"")
            ipc_addr = f"ipc:///tmp/roslike_{topic}"
            sock.connect(ipc_addr)
            self.sub_sockets[topic] = sock
            print(f"[{self.node_name}] sub connected to {ipc_addr}")

        self.callbacks[topic] = callback

    def _run_receiver(self):
        """수신 전용 루프 (C++의 run_receiver 역할)"""
        while self.running:
            # 소켓 리스트 복사해서 순회 (중간에 추가될 수 있으므로)
            for topic, sock in list(self.sub_sockets.items()):
                try:
                    # 비차단 모드로 수신
                    msg = sock.recv(flags=zmq.NOBLOCK)

                    if topic in self.callbacks:
                        #
                        # 수신 즉시 스레드풀에 언팩 및 콜백 실행 던짐
                        self.executor.submit(self._execute_callback, topic, msg)
                except zmq.Again:
                    continue  # 받은 메시지 없음
                except Exception as e:
                    print(f"Receiver Error: {e}")

            time.sleep(0.001)  # CPU 점유율 방지

    def _execute_callback(self, topic, data):
        """스레드풀에서 실행될 언팩 + 콜백 로직"""
        try:
            de = Deserializer(data)
            # 아까 만든 unpack_all로 모든 인자 추출
            args = de.unpack_all()

            # 유저가 등록한 콜백 함수 호출
            self.callbacks[topic](*args)
        except Exception as e:
            print(f"Callback Execution Error on topic [{topic}]: {e}")

    def connect(self, topic, addr, is_sub=True):
        """소켓 실제 연결 (update_socket 역할)"""
        if is_sub and topic in self.sub_sockets:
            self.sub_sockets[topic].connect(addr)
            print(f"[{self.node_name}] Sub connected to {addr}")
        elif not is_sub and topic in self.pub_sockets:
            self.pub_sockets[topic].bind(addr)  # Pub은 보통 Bind
            print(f"[{self.node_name}] Pub bound to {addr}")

    def destroy(self):
        self.running = False
        self.executor.shutdown()
        self.context.term()