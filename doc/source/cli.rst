
Command line interface
---------------------------

There are two commands available from the command line interface: ``arkcat`` and ``arkget``.


Their ``--help`` reads:


.. code-block:: shell

        $ arkcat --help
        usage: arkcat [--help] [--[no_]delim] [--[no_]whitespace] [--[no_]open_tables [--include file]* [--cfg line]* [--flatten]

            --help              : print this message
            --delim             : add outer {} or [] to top-level list or table
            --whitespace        : enhance human readability
            --width INT         : set linewrap threshold
            --flatten           : ouput in 'dotted' notation rather than 'tabular' notation
            --open_tables       : print tables as key{...} rather than key={...}
            --include file      : Include this file as a table
            --cfg line          : parse given line as a table (see below)
            --cfg -             : (special case) parse stdin as a table

        Build an ark from include files and explicit config keys



.. code-block:: shell

    $ arkget --help
    usage: arkget[[--Option | outputKeyPath] ... ]
    Build an ark from include files and explicit config keys.
    --include and --cfg args are parsed first
    The remaining arguments are processed in order.
    OutputKeyPaths are looked up in the ark and are printed using the delim and whitespace flags in effect at
    the point that the key appears on the command line
    Option:
        --help              : Print this message
        --[no_]delim        : Strip outer {} or "" (default no_delim)
        --[no_]whitespace   : Enhance human readability (default whitespace)
        --[no_]open_tables   : print tables as key{...} rather than key={...}
        --width INT         : set linewrap threshold
        --include file      : Include this file (many)
        --cfg keypath=value : Insert this path.to.key=value (many)
        outputKeyPath       : Search for this keypath in the resulting ark.
                              Print the value to stdout, followed by a newline.
                              If not specified or if zero length, print the whole ark

