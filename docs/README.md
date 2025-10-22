DOCUMENTOS REQUERIDOS EN ESTA CARPETA
========================================

Esta carpeta contiene la documentación final del proyecto orientada
al usuario final (no técnica).

ESTRUCTURA:
-----------
/fuentes/
  - enlaces-documentos.md : Enlaces a archivos editables en Drive/OneDrive

/publicados/  (Solo en releases)
  - manual-usuario.pdf : Manual de usuario final
  - ficha-tecnica.pdf : Especificaciones técnicas del dispositivo
  - diagrama-conexiones.pdf : Diagrama de conexionado (opcional)

IMPORTANTE:
-----------
- Los archivos editables (.docx, .xlsx) están en Drive/OneDrive
  (ver enlaces-documentos.md para acceder)

- Los PDFs finales solo se agregan en releases oficiales
  (durante desarrollo, /publicados/ estará vacía)

- Personal no técnico puede editar documentos en Drive
  Se sincronizan a PDFs antes de cada release

WORKFLOW:
---------
1. Edición continua → Drive/OneDrive (personal no técnico)
2. Pre-release → Generar PDFs desde Drive (equipo técnico)
3. Release → Copiar PDFs a /publicados/
4. Git commit + tag + GitHub Release

Última actualización: 2025-10-11
Mantenido por: @ja23mx