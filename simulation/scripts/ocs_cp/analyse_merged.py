import importlib.util, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("analyse_items", os.path.join(HERE, "..", "ocs", "analyse_items.py"))
ai = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ai)
ai.HERE = os.path.abspath(sys.argv[1])
for item in sys.argv[2:]:
    print(f"\n{item}\n{'=' * len(item)}\n")
    getattr(ai, "item_" + item)()
