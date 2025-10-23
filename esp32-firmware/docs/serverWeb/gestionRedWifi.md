# Gestión de Redes WiFi 

## Descripción
Administracion de las redes WiFi almacenadas en el dispositivo, incluyendo operaciones de guardado, consulta y eliminación de configuraciones de red.

## Acceso al Sistema
**URL de acceso:** `http://192.168.4.1/cmd`

## Comandos Disponibles

### 1. Guardar Red WiFi
**Comando:** `set1RedWifi:NOMBRE_RED,CLAVE,`

**Descripción:** Almacena una nueva configuración de red WiFi en el dispositivo.

**Sintaxis:**
```
set1RedWifi:<SSID>,<PASSWORD>,
```

**Parámetros:**
- `SSID`: Nombre de la red WiFi
- `PASSWORD`: Contraseña de la red WiFi
- **Importante:** El comando debe terminar con coma (`,`)

**Ejemplos:**
```
set1RedWifi:MiRedCasa,miPassword123,
set1RedWifi:RedOficina,claveSegura456,
```

### 2. Obtener Redes WiFi Guardadas
**Comando:** `getRedesWifi`

**Descripción:** Consulta todas las redes WiFi almacenadas en el dispositivo.

**Sintaxis:**
```
getRedesWifi
```

**Respuesta:** Lista de todas las redes WiFi configuradas en el sistema.

### 3. Borrar Red WiFi
**Comando:** `borrar1RedWifi:NOMBRE_RED`

**Descripción:** Elimina una configuración específica de red WiFi del dispositivo.

**Sintaxis:**
```
borrar1RedWifi:<SSID>
```

**Parámetros:**
- `SSID`: Nombre exacto de la red WiFi a eliminar

**Ejemplos:**
```
borrar1RedWifi:REDLOCAL
borrar1RedWifi:INTERNETLOCAL
borrar1RedWifi:RED2
```

## Casos de Uso

### Configuración Inicial
1. Acceder a `http://192.168.4.1/cmd`
2. Guardar red principal: `set1RedWifi:RedPrincipal,claveSegura,`
3. Verificar guardado: `getRedesWifi`

### Mantenimiento de Redes
1. Consultar redes actuales: `getRedesWifi`
2. Eliminar redes obsoletas: `borrar1RedWifi:RedVieja`
3. Agregar nuevas redes según sea necesario

## Notas Importantes
- Todos los comandos son sensibles a mayúsculas y minúsculas
- El comando `set1RedWifi` debe terminar con coma (`,`)
- Los nombres de red deben coincidir exactamente para el borrado
- Se recomienda verificar con `getRedesWifi` después de cada operación

## Limitaciones
- Verificar el límite máximo de redes WiFi que puede almacenar el dispositivo
- Las contraseñas con caracteres especiales pueden requerir codificación específica