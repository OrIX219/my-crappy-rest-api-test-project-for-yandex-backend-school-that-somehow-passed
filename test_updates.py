from tests_shared import *

UPDATES_DATE = "2020-12-12T12:15:00Z"

IMPORT_BATCHES = [
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "f1",
                "parentId": None
            },
            {
                "type": "FILE",
                "id": "e0",
                "parentId": "f1",
                "size": 128,
                "url": "/goat/se"
            }
        ],
        "updateDate": "2020-12-11T12:00:00Z"
    },
    {
        "items": [
            {
                "type": "FOLDER",
                "id": "f2",
                "parentId": "f1",
            },
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "f2",
                "size": 256
            }
        ],
        "updateDate": "2020-12-11T18:00:00Z"
    },
    {
        "items": [
            {
                "type": "FILE",
                "id": "e0",
                "parentId": "f1",
                "size": 4096,
                "url": "/x/x/x"
            },
            {
                "type": "FILE",
                "url": "/file/url4",
                "id": "e3",
                "parentId": "f1",
                "size": 512
            }
        ],
        "updateDate": "2020-12-12T02:02:00Z"
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url4",
                "id": "e3",
                "parentId": "f2",
                "size": 1024
            }
        ],
        "updateDate": "2020-12-12T12:30:00Z"
    }
]

EXPECTED_TREE = [
    {
        "items": []
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "f2",
                "size": 256,
                "date": "2020-12-11T18:00:00Z"
            }
        ]
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "f2",
                "size": 256,
                "date": "2020-12-11T18:00:00Z"
            },
            {
                "type": "FILE",
                "id": "e0",
                "parentId": "f1",
                "size": 4096,
                "url": "/x/x/x",
                "date": "2020-12-12T02:02:00Z"
            },
            {
                "type": "FILE",
                "url": "/file/url4",
                "id": "e3",
                "parentId": "f1",
                "size": 512,
                "date": "2020-12-12T02:02:00Z"
            }
        ]
    },
    {
        "items": [
            {
                "type": "FILE",
                "url": "/file/url2",
                "id": "e2",
                "parentId": "f2",
                "size": 256,
                "date": "2020-12-11T18:00:00Z"
            },
            {
                "type": "FILE",
                "id": "e0",
                "parentId": "f1",
                "size": 4096,
                "url": "/x/x/x",
                "date": "2020-12-12T02:02:00Z"
            },
        ]
    }
]

def test_updates():
    for index, batch in enumerate(IMPORT_BATCHES):
        print(f"Importing batch {index}")
        request("/imports", method="POST", data=batch)
        params = urllib.parse.urlencode({
            "date": UPDATES_DATE
        })
        status, response = request(f"/updates?{params}", json_response=True)
        assert status == 200, f"Expected HTTP status code 200, got {status}"
        expected = EXPECTED_TREE[index]
        if response != expected:
            print_diff(expected, response)
            print("Response tree doesn't match expected tree.")
            sys.exit(1)

    print("Test /updates passed.")
    status, _ = request(f"/delete/f1?date={UPDATES_DATE}")

if __name__ == "__main__":
    test_updates()
