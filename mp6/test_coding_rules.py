def ast_imports(file, allowed):
    import ast
    wast = ast.parse(open(file,'r').read())
    for node in ast.walk(wast):
        if isinstance(node, ast.Import):
            for name in node.names:
                assert name.name in allowed
        if isinstance(node, ast.ImportFrom):
            assert node.module in allowed
        if isinstance(node, ast.Name):
            assert node.id != '__import__'
            assert 'attr' not in dir(node) or node.attr == '__import__'

def test_wallet_imports():
    ast_imports('wallet.py', ['threading'])

def test_wallet_server_imports():
    ast_imports('wallet-server.py', ['threading','socket','getopt','sys','wallet'])

def test_no_globals():
    import wallet
    modtype = type(wallet)
    functype = type(test_no_globals)
    w = wallet.Wallet()
    w.change('demo',1)
    w.get('demo')
    w.get('nothing')
    ok = [name for name in globals().keys() if name.startswith('__') and name.endswith('__')]
    for name in dir(wallet):
        is_ok = name in ok or type(wallet.__dict__[name]) in (type, modtype, functype)
        assert is_ok, 'global variable "'+name+'" prohibited'
