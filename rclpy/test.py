import time
from rclpy import Node


def main():
    # 1. 노드 생성
    node = Node("python_pub_node")

    # 2. 주소 바인딩 (여기서 소켓이 생성되고 딕셔너리에 들어감)
    # 'test_topic'이라는 이름으로 tcp://*:5555에 바인드
    node.publish("test_topic", 1, 1.2, "fist", [[]])

    node.connect("test_topic", "tcp://*:5555", is_sub=False)

    print("Python Publisher started. Sending data every 1s...")

    count = 0
    while True:
        count += 1
        test_str = f"Hello from Python! [{count}]"
        test_float = 3.141592
        test_vec= [[1,2,3],[4,5,6],[7,8,9]]
        # 3. 데이터 발사 (int, float, string 3개를 한 번에 팩킹)
        node.publish("test_topic", count, test_float, test_str,test_vec)

        print(f"Published: {count}, {test_float}, {test_str}")
        time.sleep(1)


if __name__ == "__main__":
    main()