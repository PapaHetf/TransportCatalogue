# TransportCatalogue
## Описание
Парсинг JSON и сохранение данных о маршрутах и остановках в справочник, создание карты маршрутов в формате SVG.    
Ответ на запрос фофрмируется в виде JSON формата. Поиск кратчайшего маршрута от одной остановки до другой.
## Standart C++ 17
Проект собирался в Visual Studio 2019. Для удобства в проекте std::cout перенаправлен в outup.txt,  
для этого зайдите в свойство проекта и в ```Свойста конфигурации/Отладка``` укажите следующий аргументы команды:
```
<input.json 2>error.txt 1>output.txt
```
Также не забудьте указать **рабочий каталог** с входными и выходными файлами и пути к хидер-файлам.
## Формат запросов
Запрос формирутеся в виде JSON формата:
```JSON
{
    "base_requests": [ {}, {} ],
    "stat_requests": [ {}, {}, {}, {} ],
    "render_settings": {},
    "routing_settings": {} 
}
```
### 1. base_requests  
```JSON
{
  "base_requests": [ {}, {} ]
}
```
Запрос **base_requests** содержит элементы двух типов для добавления данных в транспортный спровочник: маршруты и остановки.  
**Маршруты:**
```JSON
{
  "type": "Bus",
  "name": "14",
  "stops": [
    "Чернышевская",
    "Полщадь Ленина"
  ],
  "is_roundtrip": true    
}
```
**Остановки:**
```JSON
{
  "type": "Stop",
  "name": "Электросети",
  "latitude": 43.598701,
  "longitude": 39.730623,
  "road_distances": {
    "Улица Дыбенко": 3000,
    "Ладожская": 4300
}
```
### 2. stat_requests
```JSON
{
 "stat_requests": [ {}, {}, {}, {} ]
}
```
Запрос **stat_requests** содержит элементы для получения информации о маршруте, остановке, кратчайшем маршурте между остановками,
а также запрос на визуализацию карты маршрута.  
**Запрос о маршруте:**
```JSON
{
  "id": 1,
  "type": "Bus",
  "name": "14"
}
```
**Ответ на запрос о маршруте:**  
```JSON
[ {
  "curvature": 2.18604,
  "request_id": 1,
  "route_length": 9300,
  "stop_count": 4,
  "unique_stop_count": 3
} ]
```
**Запрос об остановке:**
```JSON
{
  "id": 1,
  "type": "Stop",
  "name": "Ладожская"
} 
```
**Ответ на запрос об остановке:**  
```JSON
[ { "buses": ["14", "22к"], "request_id": 1 } ]
```
**Запрос на построение кратчайшего маршрута между двумя остановками:**
```JSON
{
  "type": "Route",
  "from": "Проспект Славы",
  "to": "Чернышевская",
  "id": 4
}
```
**Ответ на запрос о кратчайшем расстояние меджду остановками:**
```JSON
[
{
  "request_id": 4,
  "total_time": 10.5,
  "items": 
    [
      {
        "type": "Wait",
        "stop_name": "Приморская",
        "time": 6
      },
      {
        "type": "Bus",
        "bus": "80",
        "span_count": 2,
        "time": 5.235
      } 
    ]
}
]
```
**Запрос на визуализацию карты маршрута:**
```JSON
{ "id": 535, "type": "Map" }
```
Карта формируется в виде SVG формата, который помещается в JSON ответ.  
Подробно посмотреть форматы запросов и ответов вы можете в файлах **output/input.json** и **output/output.txt**.  
В случае, если маршрут или остановка не найдены, то вывод следующий:
```JSON
[ { "request_id": 12345, "error_message": "not found" } ]
```
### 3. **render_settings**
```JSON
{
"render_settings": {
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

    "color_palette": [
      "green",
      [255, 160, 0],
      "red"
    ]
  }
} 
```
Для визуализацией карты маршрута нужно добавить запрос **render_settings** и параметры для отрисовки.
### 4. **routing_settings**
```Json
{
  "routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
  } 
} 
```
Параметры для поиска кратчайшего расстояния по маршруту.  
bus_wait_time — время ожидания автобуса на остановке, в минутах.  
bus_velocity — скорость автобуса, в км/ч. 

