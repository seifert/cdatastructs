
import array
import random

from datastructs.hashmap import Int2Int

from example.computer import sum

IDS_COUNT = 1000


def test_computer():
    # Random ids
    ids = random.sample(range(1, IDS_COUNT + 1, 1), k=IDS_COUNT)
    # Two random values for each id
    x = [random.random() for unused in range(len(ids))]
    y = [random.random() for unused in range(len(ids))]
    # Compute only random selection from all ids
    ids_selection = set(random.sample(ids, random.randint(1, len(ids))))

    # Prepare pure C structures for computing
    id2position = Int2Int(IDS_COUNT)
    for position, item_id in enumerate(ids):
        id2position[item_id] = position
    a = array.array('d', x)
    b = array.array('d', y)
    result = array.array('d', (0.0 for unused in range(len(ids))))
    s = array.array('Q', ids_selection)

    # Compute data
    sum(s, id2position.get_ptr(), a, b, result)

    expected = [0.0 for unused in range(len(ids))]
    for i, item_id in enumerate(ids):
        if item_id in ids_selection:
            expected[i] = x[i] + y[i]

    assert expected == list(result)
