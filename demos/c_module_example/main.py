
import array
import math
import random

from cdatastructs.hashmap import Int2Int

from .computer import sum

IDS_COUNT = 1000


def main():
    # Random ids
    ids = random.sample(range(1, IDS_COUNT + 1, 1), k=IDS_COUNT)
    # Fill mapping ID to position
    id2position = Int2Int()
    for position, item_id in enumerate(ids):
        id2position[item_id] = position

    # Random values for calculate
    a = array.array('d', (random.random() for unused in ids))
    b = array.array('d', (random.random() for unused in ids))
    # Random ids for calculate
    ids_for_calculate = array.array(
        'Q', random.choices(ids, k=random.randint(1, len(ids))))
    # Array for results
    result = array.array('d', (math.nan for unused in ids))

    # Compute data
    sum(ids_for_calculate, id2position.buffer_ptr, a, b, result)

    for position, item_id in enumerate(ids):
        if item_id in ids_for_calculate:
            assert result[position] == a[position] + b[position]
        else:
            assert math.isnan(result[position])
    print('PASS')
