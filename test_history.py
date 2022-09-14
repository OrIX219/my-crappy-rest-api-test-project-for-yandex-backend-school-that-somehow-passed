from tests_shared import *

# normal history
# before dateStart
# on dateEnd
# folder history after content change
# history after delete

DATE_START = "2020-12-01T00:00:00Z"
DATE_END = "2020-12-02T00:00:00Z"

IMPORT_BATCHES = [
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None
            },
            {
                "type": "FILE",
                "id": "e1",
                "size": 10,
                "url": "/goat/se",
                "parentId": None
            },
            {
                "type": "FILE",
                "id": "e2",
                "size": 10,
                "url": "/goat/se",
                "parentId": None
            }
        ],
        "updateDate": "2020-11-11T12:00:00Z"
    },
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None
            },
            {
                "type": "FILE",
                "id": "e1",
                "parentId": "root",
                "size": 100,
                "url": "/goat/se"
            }
        ],
        "updateDate": "2020-12-01T00:00:00Z"
    },
    {
        "items": [

            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "root",
                "size": 200
            }
        ],
        "updateDate": "2020-12-01T12:00:00Z"
    },
    {
        "items": [
            {
                "type": "FILE",
                "id": "e2",
                "parentId": "root",
                "size": 300,
                "url": "/x/x/x"
            }
        ],
        "updateDate": "2020-12-01T22:00:00Z"
    },
]

EXPECTED_ROOT = [
    {
        "items": []
    },
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 0,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 100,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            }
        ]
    },
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 0,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 100,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 300,
                "date": "2020-12-01T12:00:00Z",
                "url": None
            }
        ]
    },
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 0,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 100,
                "date": "2020-12-01T00:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 300,
                "date": "2020-12-01T12:00:00Z",
                "url": None
            },
            {
                "type": "FOLDER",
                "id": "root",
                "parentId": None,
                "size": 400,
                "date": "2020-12-01T22:00:00Z",
                "url": None
            }
        ]
    }
]

EXPECTED_E1 = [
    {
        "items": []
    },
    {
        "items": [
            {     
                "type": "FILE",
                "id": "e1",
                "parentId": "root",
                "size": 100,
                "url": "/goat/se",
                "date": "2020-12-01T00:00:00Z"
            }
        ]
    },
    {
        "items": [
            {
                "type": "FILE",
                "id": "e1",
                "parentId": "root",
                "size": 100,
                "url": "/goat/se",
                "date": "2020-12-01T00:00:00Z"
            },
        ]
    },
    {
        "items": [
            {
                "type": "FILE",
                "id": "e1",
                "parentId": "root",
                "size": 100,
                "url": "/goat/se",
                "date": "2020-12-01T00:00:00Z"
            },
        ]
    },
]

EXPECTED_E2 = [
    {
        "items": []
    },
    {
        "items": []
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "root",
                "size": 200,
                "date": "2020-12-01T12:00:00Z"
            }
        ]
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "root",
                "size": 200,
                "date": "2020-12-01T12:00:00Z"
            },
            {
                "type": "FILE",
                "id": "e2",
                "parentId": "root",
                "size": 300,
                "url": "/x/x/x",
                "date": "2020-12-01T22:00:00Z"
            }
        ]
    }
]

def test_history():
    for index, batch in enumerate(IMPORT_BATCHES):
        print(f"Importing batch {index}")
        status, _ = request("/imports", method="POST", data=batch)
        assert status == 200, f"Expected HTTP status code 200, got {status}"
        params = urllib.parse.urlencode({
          "dateStart": DATE_START,
          "dateEnd": DATE_END
        })
        status, res_root = request(f"/node/root/history?{params}", json_response=True)
        assert status == 200, f"Expected HTTP status code 200, got {status}"
        expected_root = EXPECTED_ROOT[index]
        deep_sort_children(res_root)
        deep_sort_children(expected_root)
        if res_root != expected_root:
            print_diff(expected_root, res_root)
            print("ROOT: Response tree doesn't match expected tree.")
            sys.exit(1)
        status, res_e1 = request(f"/node/e1/history?{params}", json_response=True)
        assert status == 200, f"Expected HTTP status code 200, got {status}"
        expected_e1 = EXPECTED_E1[index]
        deep_sort_children(res_e1)
        deep_sort_children(expected_e1)
        if res_e1 != expected_e1:
            print_diff(expected_e1, res_e1)
            print("E1: Response tree doesn't match expected tree.")
            sys.exit(1)
        status, res_e2 = request(f"/node/e2/history?{params}", json_response=True)
        assert status == 200, f"Expected HTTP status code 200, got {status}"
        expected_e2 = EXPECTED_E2[index]
        deep_sort_children(res_e2)
        deep_sort_children(expected_e2)
        if res_e2 != expected_e2:
            print_diff(expected_e2, res_e2)
            print("E2: Response tree doesn't match expected tree.")
            sys.exit(1)

    status, _ = request(f"/delete/root?date={DATE_END}", method="DELETE")
    assert status == 200, f"Expected HTTP status code 200, got {status}"

    status, _ = request(f"/node/root/history?dateStart={DATE_START}&dateEnd={DATE_END}", json_response=True)
    assert status == 404, f"Expected HTTP status code 404, got {status}"

    print("Test /node/{id}/history passed.")

if __name__ == "__main__":
    test_history()