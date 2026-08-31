# Pipeline del Prof — Elaborato CG: AI-Powered Texture and Material Generation for 3D Rendering

*(Ricostruito dal messaggio `message.txt` incollato da Emanuele il 05/08/2026. Il file .txt originale è stato purgato dalla cache; questo file ne conserva il contenuto completo.)*

## Workflow

1. L'utente seleziona o descrive un materiale (es. "muro di mattoni antichi").
2. Un modello generativo (Stable Diffusion o ControlNet) crea la texture.
3. Vengono generate anche le mappe PBR (normal, roughness, displacement) oppure derivate automaticamente.
4. La texture viene applicata in tempo reale a un modello 3D renderizzato con OpenGL.
5. L'utente può confrontare il rendering con e senza materiali generati dall'IA, valutando qualità visiva e prestazioni.

Il progetto permette di trattare:
- pipeline di rendering (texture mapping, materiali PBR, illuminazione)
- integrazione di modelli di IA generativa
- valutazione delle prestazioni (FPS, tempi di inferenza)
- confronto qualitativo dei risultati

## Nota su PBR (Physically Based Rendering)

I PBR sono lo standard per il rendering realistico nei motori grafici come Unity, Unreal Engine, Blender, Godot e nei renderer professionali. Invece di "dipingere" manualmente come un oggetto riflette la luce, si descrivono le proprietà fisiche del materiale, e il motore grafico calcola come la luce interagisce con esso.

**Mappe PBR principali:**
1. **Albedo (Base Color)** — Il colore puro del materiale, senza ombre né riflessi (es. mattoni → rosso; legno → marrone; neve → bianco). La texture "classica".
2. **Normal Map** — Non modifica realmente la geometria, ma fa sembrare la superficie piena di piccoli dettagli (es. un muro di mattoni appare irregolare anche se il modello è perfettamente piatto).
3. **Roughness Map** — Controlla la rugosità della superficie: valori bassi = specchio, metallo lucidato, vetro; valori alti = cemento, carta, pietra. La roughness influenza la nitidezza dei riflessi.
4. **Metallic Map** — Quasi binaria: 0 = non metallo, 1 = metallo (metalli: ferro, oro, rame, alluminio; non metalli: legno, plastica, pelle, carta).
5. **Ambient Occlusion (AO)** — Rappresenta le zone dove arriva meno luce (fessure, angoli, cavità). Aggiunge profondità senza aumentare la complessità del modello.
6. **Height o Displacement Map** — Descrive vere variazioni di altezza. Può modificare la geometria (displacement) o simulare profondità (parallax mapping). Usata per rocce, mattoni, corteccia, terreno.

Un materiale PBR tipico contiene: Albedo, Normal, Roughness, Metallic, AO, Height.

**Dove entra l'IA:** Da una singola foto (o immagine generata da un modello come Stable Diffusion), l'IA genera automaticamente: foto → Albedo → Normal Map → Roughness → Height Map → Metallic Map. Un materiale PBR completo, pronto per un motore grafico.

## Partes del proyecto

**Parte IA:**
- Generazione di texture con Stable Diffusion o modelli simili.
- Generazione automatica di mappe PBR (normal, roughness, height) con modelli dedicati o reti neurali.

**Parte CG:**
- Caricamento delle texture nel motore grafico.
- Implementazione di uno shader PBR (oppure utilizzo di uno standard già disponibile in OpenGL).
- Confronto visivo tra un materiale semplice e uno PBR sotto diverse condizioni di illuminazione.

## Riferimenti (ordine di lettura consigliato)

1. **OpenGL e shader base (LearnOpenGL)** — Il punto di partenza più accessibile. Prima della sezione PBR, conoscere almeno: illuminazione Phong, normal mapping, gamma correction, HDR, cubemap, framebuffer. La sezione PBR introduce il modello a microfaccette, la conservazione dell'energia e l'equazione Cook-Torrance. Ordine consigliato: Normal Mapping → Gamma Correction → HDR → PBR Theory → PBR Lighting → IBL Diffuse Irradiance → IBL Specular IBL.
2. **Guida PBR di Google Filament** — Una delle migliori risorse tecniche sul PBR real-time. Spiega BRDF e modello Cook-Torrance, GGX, Fresnel, geometric shadowing, metallic/roughness, image-based lighting, tone mapping, gestione del colore, clear coat. Più rigorosa di LearnOpenGL.
3. **Paper Disney sul PBR** — Brent Burley, "Physically-Based Shading at Disney". Descrive un modello sviluppato studiando materiali reali. Importante per: significato della roughness, risposta speculare, Fresnel, comportamento dei materiali metallici e dielettrici. Lettura avanzata, consigliata dopo LearnOpenGL.
4. **Specifiche glTF 2.0** — Per decidere come rappresentare e caricare i materiali. Adotta un modello PBR metallic-roughness e specifica precisamente il significato dei parametri. Importante per capire il packing dei canali (occlusion, roughness e metallic possono essere memorizzati in canali diversi delle texture).
5. **Blender Principled BSDF** — Utile per confrontare il renderer OpenGL sviluppato con un renderer già funzionante. Creare un materiale in Blender, assegnargli le stesse texture usate in OpenGL, usare condizioni di illuminazione simili, confrontare i risultati. Le immagini non saranno perfettamente identiche (Blender usa modelli/approssimazioni diverse), ma il confronto serve come controllo qualitativo.

**Riferimenti per la parte IA:**
6. **Latent Diffusion Models** — Rombach et al., "High-Resolution Image Synthesis with Latent Diffusion Models". Il lavoro alla base della famiglia Stable Diffusion. Per il progetto non occorre riprodurre il modello: è sufficiente comprenderne encoder/decoder, spazio latente, processo di denoising, conditioning testuale. Usare un modello preaddestrato per generare la texture RGB iniziale.
7. **ControlNet** — Zhang, Rao, Agrawala, "Adding Conditional Control to Text-to-Image Diffusion Models". Condizionare un modello di diffusione mediante informazioni spaziali (bordi, profondità, segmentazione, pose). Potrebbe essere usato per conservare la struttura di uno schizzo, controllare venature del legno, mantenere geometria dei mattoni. Estensione interessante, non indispensabile per la prima versione.
8. **Material Palette** — Lopes, Pizzati, de Charette, "Material Palette: Extraction of Materials from a Single Image". Propone di estrarre materiali PBR da una singola immagine usando modelli di diffusione e decomposizione in proprietà SVBRDF. Utile per comprendere la pipeline: immagine RGB → riconoscimento materiale → texture → mappe PBR → rendering. Il termine SVBRDF (Spatially Varying BRDF) indica che le proprietà del materiale variano da punto a punto sulla superficie.
9. **SuperMat** — Affronta la decomposizione di una singola immagine in mappe albedo, metallic e roughness, con attenzione alla coerenza fisica e inferenza interattiva. Più avanzato, può essere usato come riferimento per discutere lo stato dell'arte. Non consigliata l'implementazione da zero.

**Libri utili:**
- *Real-Time Rendering* — Akenine-Möller, Haines, Hoffman et al. Riferimento generale su pipeline grafica, shading, BRDF, texture, ombre, global illumination, gestione del colore, rendering real-time.
- *Physically Based Rendering: From Theory to Implementation* — Pharr, Jakob, Humphreys. Testo rigoroso su radiometria, materiali, BRDF e luce.
- *OpenGL Programming Guide* — Utile per la parte OpenGL: shader, buffer, texture, framebuffer.

## Ordine consigliato (pipeline)

- **Fase 1 — Renderer:** LearnOpenGL normal mapping e gamma correction. LearnOpenGL PBR Theory. LearnOpenGL PBR Lighting. Implementazione albedo, normal, roughness, metallic e AO. Confronto con Blender.
- **Fase 2 — Illuminazione realistica:** LearnOpenGL diffuse irradiance. LearnOpenGL specular IBL. Guida Filament per verificare formule e convenzioni. Tone mapping ed esposizione.
- **Fase 3 — IA:** Paper sui Latent Diffusion Models. Generazione di una texture RGB mediante un modello preaddestrato. Produzione delle mappe PBR con un modello esistente o pipeline semplificata. Lettura di Material Palette per confrontare con un approccio di ricerca.
- **Fase 4 — Valutazione:** Renderizzare lo stesso oggetto con materiali diversi. Cambiare illuminazione e punto di vista. Valutare continuità della texture, realismo, tempi di generazione e FPS. Confrontare mappe create manualmente, mappe generate dall'IA e mappe di riferimento.

Versione possibile: generazione IA della texture albedo, stima automatica di normal e roughness, applicazione con shader Cook-Torrance OpenGL e valutazione sotto più ambienti HDR.

---
*Salvato in: /opt/data/cg-elaborato/pipeline-prof.md*
