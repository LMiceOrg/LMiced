import qbs 1.0

Project {
    minimumQbsVersion: "1.7.1"
    condition:qbs.targetOS.contains("macos")
    name:"科学计算与系统仿真环境系统"
    StaticLibrary {
        Depends {name:"cpp"}
        name:"2.环境抽象库"
        files:["eal/*.h", "eal/*.c"]
    }
    CppApplication {
        name:"3.公共平台"
        consoleApplication: true
        files: "main.cpp"
        Depends
        {
            name:"cpp"
        }
        Depends
        {
            name:"2.环境抽象库";cpp.link:false
        }

        Group {     // Properties for the produced executable
            fileTagsFilter: product.type
            qbs.install: true
        }
    }
    LoadableModule {
        name:"0.文档"
        //condition: qbs.buildVariant === "debug"
        files:"README.md"


    }

    Product {
        name: "1. 数据存储子系统"
        type: "LoadableModule"

    }
    Product {
        name: "5.网页控制台"
        files:"portal/*.*"

    }
    CppApplication {
        name: "4.中间件软件"
        files:"portal/*.*"

    }
}
