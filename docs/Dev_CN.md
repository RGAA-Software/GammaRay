#### 1.qwindowkit
##### qwindowkit使用在带有opengl的窗口上时，只能自己使用qwindowkit，其他窗口无法使用

#### 2.sqlite_orm
> 新增字段时，需要制定默认值，否则sync_schema会将整个表drop
```cpp
make_column("test", &StreamItem::test_, default_value(0))
```

