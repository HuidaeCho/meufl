# Memory-Efficient Upstream Flow Length (MEUFL)

Part of the [Memory-Efficient I/O-Improved Drainage Analysis System (MIDAS)](https://github.com/HuidaeCho/midas)

**Citation**: Huidae Cho, 2026. Manuscript Under Review. Flow in Float: Memory-Efficient Upstream Flow Length Parallel Computation Using an IEEE-754-Based Union Encoding. Environmental Modelling & Software.

Predefined flow direction encodings in GeoTIFF: power2 (default, r.terraflow, ArcGIS), taudem (d8flowdir), 45degree (r.watershed), degree<br>
![image](https://github.com/user-attachments/assets/990f0530-fded-4ee5-bfbb-85056a50ca1c)
![image](https://github.com/user-attachments/assets/a02dfc15-a825-4210-82c4-4c9296dafadc)
![image](https://github.com/user-attachments/assets/64f5c65a-c7cc-4e06-a69f-6fccd6435426)
![image](https://github.com/user-attachments/assets/fafef436-a5f2-464a-89a8-9f50a877932c)

Custom flow direction encoding is also possible by passing `-e E,SE,S,SW,W,NW,N,NE` (e.g., 1,8,7,6,5,4,3,2 for taudem).

## Test data

You can find test data at https://data.isnew.info/meufl.html.
