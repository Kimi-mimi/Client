# Клиентская часть курсача по SMTP
Однопоточный, однопроцессный, с использованием `select`. 
Логгирование в отдельном процессе. 

* Запуск клиента
    ```console
    foo@bar:~$ make main
    foo@bar:~$ build/main [loopback/no-loopback]
    ```

* Для запуска тестов (используется CUnit)
    ```console
    foo@bar:~$ make test
    foo@bar:~$ build/test/test
    ```

* Для сборки отчета
    ```console
    foo@bar:~$ make report
    ```

* Для сборки всего сразу
    ```console
    foo@bar:~$ make
    ```